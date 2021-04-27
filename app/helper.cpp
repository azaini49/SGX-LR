#include "helper.h"
#include <thread>
#include <fstream>
#include <math.h>
#include <iostream>
#include "../include/Queue.h"
#include <mutex>
#include <map>
#include <cmath>
#include "../include/sync_utils.hpp"
#include "../tools/gmpxx.h"


extern Queue* task_queue;
static std::map<mpz_class, mpz_class> lookup;
std::mutex gaurd;

int sigmoid(mpz_t res, double x)
{
    float expVal;
    float result;

    // Compute the exponent
    expVal = exp(-x);

    // Compute the sigmoid function
    result = 1 / (1 + expVal);
    double tmp = ceil(result);
    if(result <= 0.5)
        tmp = floor(result);
    mpz_set_si(res, tmp);
    return COMPLETED;
}

// Utility function to generate lookup table
void lookup_table_util(mpz_class limit, std::shared_ptr<Context> ctx, int tid, int numThreads)
{
    // Temporary variable
    mpz_t tmp;
    mpz_init(tmp);

    mpz_class i = tid;
    while(i < limit)
    {
        mpz_powm(tmp, ctx->g, i.get_mpz_t(), ctx->p);
        std::unique_lock<std::mutex> locker(gaurd);
        lookup[mpz_class{tmp}] = i;
        mpz_invert(tmp, tmp, ctx->p);
        lookup[mpz_class{tmp}] = -i;
        locker.unlock();
        i = i + numThreads;
    }
    mpz_clear(tmp);
}

// Precompute the lookup table
void compute_lookup_table(std::shared_ptr<Context> ctx, int bits)
{
    if(ctx == NULL)
    {
        std::cout << "Context is NULL (Lookup table construction)\n";
        exit(1);
    }
    mpz_t tmp;
    mpz_init(tmp);

    mpz_class i = 0;
    mpz_class new_m = pow(2, bits);

    // Define threadpool
    int numThreads = std::thread::hardware_concurrency();
    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(lookup_table_util, new_m, ctx, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
    mpz_class k = 1;
    mpz_clear(tmp);
}

// Straight forward lookup in lookup table
void get_discrete_log(mpz_t x, std::shared_ptr<Context> ctx)
{
    mpz_class key{x};
    std::map<mpz_class, mpz_class, MapComp>::iterator it = lookup.find(key);
    if(it == lookup.end())
    {
        mpz_invert(x, x, ctx->p);
        mpz_class key{x};
        it = lookup.find(key);
        if(it == lookup.end())
            mpz_set_si(x, -1);
        else
            mpz_mul_si(x, it->second.get_mpz_t(), -1);
    }
    else
        mpz_set(x, it->second.get_mpz_t());
}

void make_request(Request req)
{
    req->status = SCHEDULED;
    task_queue->enqueue(req);
    while(true)
    {
        if(req->status == SCHEDULED)
            __asm__("pause");
        else
        {
            if(req->status == ERROR)
            {
                spin_unlock(&req->status);
                exit(EXIT_FAILURE);
            }
            spin_unlock(&req->status);
            break;
        }
    }
    // Reset request. Do not free data!!
    req->status = IDLE;
}

/**
 * Compute the hamming distance of the rows of A and B specified by 'row'
 */
void compute_hamming_distance(Matrix res, const Matrix A, const Matrix B, int row, int A_c1, int A_c2, int B_c1, int B_c2)
{
    if(row < 0 || row >= A->rows || row >= B->rows)
    {
        std::cout << "A(" << A->rows << ", " << A->cols << ")\n";
        std::cout << "B(" << B->rows << ", " << B->cols << ")\n";
        std::cout << "Requested row : " << row << std::endl;
        throw std::invalid_argument("Invalid dimensions!");
    }
    if(A_c2 == -1)
        A_c2 = A->cols - 1;
    if(B_c2 == -1)
        B_c2 = B->cols - 1;
    if(A_c2 - A_c1 != B_c2 - B_c1)
    {
        std::cout << "A : (" << A_c1 << ", " << A_c2 << ") : " << A_c2 - A_c1 + 1 << std::endl;
        std::cout << "B : (" << B_c1 << ", " << B_c2 << ") : " << B_c2 - B_c1 + 1 << std::endl;
        throw std::invalid_argument("[CHD] Invalid dimensions!");
    }

    int range = A_c2 - A_c1 + 1;
    if(range != res->cols)
        throw std::invalid_argument("[CHD] Invalid dimensions (res)!");

    for(int i = 0; i < range; i++)
    {
      // 1, 0, -1
        if(mpz_cmp(mat_element(A, row, A_c1 + i), mat_element(B, row, B_c1 + i)) == 0 && mat_element(A, row, A_c1 + i) == 0)
            mpz_set_si(mat_element(res, row, i), 0);
        else
            mpz_set_si(mat_element(res, row, i), 1);
    }
}

void compute_stat_distance(Matrix res, const Matrix A, const Matrix B, int row, int A_c1, int A_c2, int B_c1, int B_c2)
{
    if(row < 0 || row >= A->rows || row >= B->rows)
    {
        std::cout << "A(" << A->rows << ", " << A->cols << ")\n";
        std::cout << "B(" << B->rows << ", " << B->cols << ")\n";
        std::cout << "Requested row : " << row << std::endl;
        throw std::invalid_argument("Invalid dimensions!");
    }
    if(A_c2 == -1)
        A_c2 = A->cols - 1;
    if(B_c2 == -1)
        B_c2 = B->cols - 1;
    if(A_c2 - A_c1 != B_c2 - B_c1)
    {
        std::cout << "A : (" << A_c1 << ", " << A_c2 << ") : " << A_c2 - A_c1 + 1 << std::endl;
        std::cout << "B : (" << B_c1 << ", " << B_c2 << ") : " << B_c2 - B_c1 + 1 << std::endl;
        throw std::invalid_argument("[CHD] Invalid dimensions!");
    }

    int range = A_c2 - A_c1 + 1;
    if(range != res->cols)
        throw std::invalid_argument("[CHD] Invalid dimensions (res)!");

    for(int i = 0; i < range; i++)
    {
      // 0 = TN, 1 = TP, 2 = FN, 3 = FP
      if(mpz_get_si(mat_element(A, row, A_c1 + i)) == 0 && mpz_get_si(mat_element(B, row, B_c1 + i)) == 0){
        mpz_set_si(mat_element(res, row, i), 0);
      }else if(mpz_get_si(mat_element(A, row, A_c1 + i)) == 1 && mpz_get_si(mat_element(B, row, B_c1 + i))  == 1){
        mpz_set_si(mat_element(res, row, i), 1);
      }else if(mpz_get_si(mat_element(A, row, A_c1 + i)) == 0 && mpz_get_si(mat_element(B, row, B_c1 + i))  == 1){
        mpz_set_si(mat_element(res, row, i), 2);
      }else if(mpz_get_si(mat_element(A, row, A_c1 + i)) == 1 && mpz_get_si(mat_element(B, row, B_c1 + i))  == 0){
        mpz_set_si(mat_element(res, row, i), 3);
      }

    }
}

void compute_vector_difference(Matrix res, const Matrix A, const Matrix B, int row, int A_c1, int A_c2, int B_c1, int B_c2)
{
    if(row < 0 || row >= A->rows || row >= B->rows)
    {
        std::cout << "A(" << A->rows << ", " << A->cols << ")\n";
        std::cout << "B(" << B->rows << ", " << B->cols << ")\n";
        std::cout << "Requested row : " << row << std::endl;
        throw std::invalid_argument("Invalid dimensions!");
    }
    if(A_c2 == -1)
        A_c2 = A->cols - 1;
    if(B_c2 == -1)
        B_c2 = B->cols - 1;
    if(A_c2 - A_c1 != B_c2 - B_c1)
    {
        std::cout << "A : (" << A_c1 << ", " << A_c2 << ") : " << A_c2 - A_c1 + 1 << std::endl;
        std::cout << "B : (" << B_c1 << ", " << B_c2 << ") : " << B_c2 - B_c1 + 1 << std::endl;
        throw std::invalid_argument("[CHD] Invalid dimensions!");
    }

    int range = A_c2 - A_c1 + 1;
    if(range != res->cols)
        throw std::invalid_argument("[CHD] Invalid dimensions (res)!");

    for(int i = 0; i < range; i++)
        mpz_sub(mat_element(res, row, i), mat_element(A, row, A_c1 + i), mat_element(B, row, B_c1 + i));
}

// Read input csv file
void readFile(std::string filename, std::vector<std::vector<int> > &contents)
{
    std::ifstream src(filename);
    if (!src)
    {
        std::cerr << "Error opening file.\n\n";
        exit(1);
    }

  std::string buffer;
  size_t strpos = 0;
  size_t endpos;
  while(getline(src, buffer))
  {
    endpos= buffer.find(',');
    int i = 0;
    std::vector<int> tmp;
    while (endpos < buffer.length())
    {
        tmp.push_back(stoi(buffer.substr(strpos,endpos - strpos)));
        strpos = endpos + 1;
        endpos = buffer.find(',', strpos);
        i++;
    }
    tmp.push_back(std::stoi(buffer.substr(strpos)));
    contents.push_back(tmp);
  }
}

// Utility function used by populate
void populate_util(Matrix inp, std::vector<std::vector<int> > xtest, int tid, int numThreads)
{
    int row = tid;
    while(row < inp->rows)
    {
        for(int col = 0; col < inp->cols; col++)
        {
            mpz_class tmp = xtest[row][col];
            //inp.set(tmp.get_mpz_t(), row, col);
            set_matrix_element(inp, row, col, tmp.get_mpz_t());
        }
        row = row + numThreads;
    }
}


// Populate the input matrix with the contents of xtest
void populate(Matrix inp, std::vector<std::vector<int> > xtest)
{
    // Define threadpool
    int numThreads = inp->rows;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::vector<std::thread> threads(numThreads);
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(populate_util, inp, ref(xtest), i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
}

Matrix deserialize_matrix(uint8_t* buff)
{
    int r;
    int c;
    int idx = 0;
    memcpy(&r, &buff[idx], sizeof(int));
    idx = idx + sizeof(int);

    memcpy(&c, &buff[idx], sizeof(int));
    idx = idx + sizeof(int);
    // Matrix_Packet packet = (Matrix_Packet)serial;
    Matrix m = mat_init(r, c);
    memcpy(m->data, &buff[idx], r * c * sizeof(mpz_t));
    return m;
}
