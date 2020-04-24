#include "helper.h"
#include <thread>
#include <fstream>
#include <math.h>
#include <iostream>
#include "../include/Queue.h"
//#include <mutex>
#include "../include/sync_utils.hpp"


extern Queue* task_queue;

void make_request(Request &req)
{
    // std::unique_lock<std::mutex> locker(req->gaurd);
    // Request* requests = make_request_copy(req);
    // for(int i = 0; i < req->num_threads; i++)
    // {
    //     //req->tid = i
    //     task_queue->enqueue(req);
    // }
    task_queue->enqueue(req);
    while(true)
    {
        if(req->status != COMPLETED || req->status != ERROR)
            __asm__("pause");
        else
        {
            std::cout << "[" << req->job_id << "] : " << req->status << std::endl;
            if(req->status == ERROR)
            {
                spin_unlock(&req->status);
                std::cout << "One or more attributs of request might be null!\n";
                exit(EXIT_FAILURE);
            }
            spin_unlock(&req->status);
            break;
        }
    }

    // Reset request. Do not free data!!
    req->status = IDLE;
    req->input_1 = NULL;
    req->output = NULL;
    req->compression = NULL;
    req->cmt = NULL;
}

/**
 * Compute the hamming distance of the rows of A and B specified by 'row'
 */
void compute_hamming_distance(Matrix &res, const Matrix &A, const Matrix &B, int row, int A_c1, int A_c2, int B_c1, int B_c2)
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
        if(mpz_cmp(mat_element(A, row, A_c1 + i), mat_element(B, row, B_c1 + i)) == 0)
            mpz_set_si(mat_element(res, row, i), 0);
        else
            mpz_set_si(mat_element(res, row, i), 1);
    }
}

void compute_vector_difference(Matrix &res, const Matrix &A, const Matrix &B, int row, int A_c1, int A_c2, int B_c1, int B_c2)
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
void populate_util(Matrix &inp, std::vector<std::vector<int> > xtest, int tid, int numThreads)
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
void populate(Matrix &inp, std::vector<std::vector<int> > xtest)
{
    // Define threadpool
    int numThreads = inp->rows;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;
    
    std::vector<std::thread> threads(numThreads);
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(populate_util, std::ref(inp), ref(xtest), i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
}

