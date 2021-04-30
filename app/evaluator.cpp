#include "evaluator.h"
#include <cmath>
#include <thread>
#include <mutex>
#include "helper.h"
#include <iostream>

/**
 * Constructor for evaluator
 * @params : context
 */
Evaluator::Evaluator(std::shared_ptr<Context> context)
    :ctx(context)
{
    if(ctx == NULL)
        throw std::invalid_argument("Context cannot be null!");
}

/**
 * Utility function to perform multithreaded compression
 */
 //basically the same as before
 void Evaluator::compress_util_IP(Evaluator &eval, Matrix compression, const Matrix ciphertext, const Matrix inp, int start, int end, int tid, int numThreads)
 {
     int row = tid;

     // Store intermediate values
     mpz_t tmp;
     mpz_init(tmp);

     while(row < end + 1)
     {
         mpz_set_si(mat_element(compression, row, 0), 1);
         //std::cout << "pre compression row: "<< row <<", col:" << 0 << ", val: "<< mpz_get_si(mat_element(compression, row, 0)) <<"\n";
         for(int col = 0; col < ciphertext->cols; col++)
         {

             mpz_powm(tmp, mat_element(ciphertext, row, col), mat_element(inp, 0, col), eval.ctx->Ns); //take cti^yi
             mpz_mul(mat_element(compression, row, 0), mat_element(compression, row, 0), tmp); // mult cti^yi times prev
             mpz_mod(mat_element(compression, row, 0), mat_element(compression, row, 0), eval.ctx->Ns); // take mod of above
             //std::cout << "pre compression row: "<< row <<", col:" << col << ", val: "<< mpz_get_si(mat_element(compression, row, 0)) <<"\n";
         }
         row = row + numThreads;
     }
     mpz_clear(tmp);
 }

/**
 * Compress the ciphertext given inp
 * @params : ciphertext, 2nd input to function being evaluated
 * @return : compressed ciphertext
 */
void Evaluator::compress(Matrix compression, const Matrix ciphertext, const Matrix inp, int start, int end)
{
    if(ciphertext->rows != compression->rows || compression->cols != 1 || inp->cols != ciphertext->cols)
    {
        std::cout << "[EXPECTED] Compression(" << ciphertext->rows << ", " << 1 << ")\n";
        std::cout << "[RECEIVED] Compression(" << compression->rows << ", " << compression->cols << ")\n";
        std::cout << "[EXPECTED] Input(" << 1 << ", " << ciphertext->cols << ")\n";
        std::cout << "[RECEIVED] Input(" << inp->rows << ", " << inp->cols << ")\n";
        throw std::invalid_argument("Invalid dimensions!!");
    }

    std::cout << "\n======== COMPRESSING =======\n";

    if(end == -1)
        end = ciphertext->rows - 1;

    // Define threadpool
    int numThreads = end - start + 1;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::vector<std::thread> threads(numThreads);
    for(int i = start; i < start + numThreads; i++)
    {
        threads[i-start] = std::thread(compress_util_IP, std::ref(*this), std::ref(compression), std::ref(ciphertext), std::ref(inp), start, end, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << "=> " << ciphertext->rows << "x" << ciphertext->cols << " Matrix of Ciphertexts Compressed to "<< compression->rows << "x"<< compression->cols<< "\n";
}

//TODO! update
void Evaluator::evaluate_util_IP(Evaluator &eval, Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t &sfk, int activation, int start, int end, int tid, int numThreads)
{
    int row = tid;
    mpz_t ct0;
    mpz_init(ct0);

    mpz_t N_inv;
    mpz_init(N_inv);

    while(row < end + 1)
    {

        mpz_set(ct0, mat_element(cmt, row, 0));
        mpz_powm(ct0, ct0, sfk, eval.ctx->Ns); // take ct0^sfk
        mpz_invert(ct0, ct0, eval.ctx->Ns); // find 1/ct0^sfk
        mpz_set_si(mat_element(dest, row, 0), 1);

        mpz_mul(mat_element(dest, row, 0), mat_element(compression, row, 0), ct0); // find compression / ct0^sfk
        mpz_mod(mat_element(dest, row, 0), mat_element(dest, row, 0), eval.ctx->Ns); // take mod of prev value

        // get DL
        mpz_sub_ui(mat_element(dest, row, 0), mat_element(dest, row, 0), 1); // subtract 1 from compression / ct0^sfk
        mpz_tdiv_q(mat_element(dest, row, 0), mat_element(dest, row, 0), eval.ctx->N); // divide compression / ct0^sfk -1  by N
        mpz_mod(mat_element(dest, row, 0), mat_element(dest, row, 0), eval.ctx->Ns); // take mod of prev value


        /*mpz_sub_ui(check, check, 1); // subtract 1 from compression / ct0^sfk
        mpz_tdiv_q(check, check, eval.ctx->N); // divide compression / ct0^sfk -1  by N
        mpz_mod(check, check, eval.ctx->Ns); // take mod of prev value

        std::cout << "check " << mpz_get_si(check) << "\n";*/

        //get_discrete_log(mat_element(dest, row, 0), eval.ctx);
        if(activation == ACTIVATION)
            sigmoid(mat_element(dest, row, 0), mpz_get_d(mat_element(dest, row, 0)));
        row = row + numThreads;
    }
    mpz_clear(ct0);
    mpz_clear(N_inv);
}


void Evaluator::evaluate(Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t &sfk, int activation, int start, int end)
{
    if(compression->rows != dest->rows || compression->rows != cmt->rows || cmt->cols != dest->cols || cmt->cols != compression->cols || cmt->cols != 1)
    {
        std::cout << "Compression(" << compression->rows << ", " << compression->cols << ")\n";
        std::cout << "Destination(" << dest->rows << ", " << dest->cols << ")\n";
        std::cout << "Commitment(" << cmt->rows << ", " << cmt->cols << ")\n";
        throw std::invalid_argument("Invalid dimensions!!");
    }

    std::cout << "\n======== DECRYPTING ========\n";

    if(end == -1)
        end = dest->rows - 1;

    // Define threadpool
    int numThreads = end - start + 1;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::vector<std::thread> threads(numThreads);
    for(int i = start; i < start + numThreads; i++)
    {
        threads[i-start] = std::thread(evaluate_util_IP, std::ref(*this), std::ref(dest), std::ref(compression), std::ref(cmt), std::ref(sfk), activation, start, end, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << "=> Inner Product Calculated\n";
}

//TODO! update
void Evaluator::cca_check_util_IP(Evaluator &eval, const Matrix b, const Matrix cca_ct, const Matrix y, mpz_t hky_cca_j1, mpz_t hky_cca_j2, int start, int end, int tid, int numThreads)
{
    int row = tid;

    while(row < end + 1)
    {

        mpz_t tmp;
        mpz_init(tmp);

        mpz_t right;
        mpz_init(right);

        mpz_add(tmp, hky_cca_j1, hky_cca_j2);
        mpz_powm(right, mat_element(b, row, 0), tmp, eval.ctx->Ns);

        mpz_t left;
        mpz_init_set_si(left, 1);

        mpz_t mult;
        mpz_init(mult);

        for(int col = 0; col < cca_ct->cols; col++)
        {
           mpz_mul(mult, mat_element(y, 0, col), mat_element(cca_ct, row, col));
           mpz_add(left, left, mult);

        }

        if (mpz_cmp(left,right) != 0)
        {
          printf("CCA Check does not pass!");
          throw std::invalid_argument("CCA Check does not pass!");
        }

        mpz_clear(right);
        mpz_clear(left);
        mpz_clear(mult);
        mpz_clear(tmp);

        row = row + numThreads;
    }



}


void Evaluator::cca_check(const Matrix cca_ct, const Matrix b, const Matrix y, mpz_t hky_cca_j1, mpz_t hky_cca_j2, int start, int end)
{
    if(cca_ct->rows != b->rows || b->cols != 1)
    {
        throw std::invalid_argument("Invalid dimensions!!");
    }

    std::cout << "\n==== CHECK CCA SECURITY ====\n";

    if(end == -1)
        end = cca_ct->rows - 1;

    // Define threadpool
    int numThreads = end - start + 1;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::vector<std::thread> threads(numThreads);
    for(int i = start; i < start + numThreads; i++)
    {
        threads[i-start] = std::thread(cca_check_util_IP, std::ref(*this), std::ref(b), std::ref(cca_ct), std::ref(y), std::ref(hky_cca_j1), std::ref(hky_cca_j2), start, end, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();


    std::cout << "=> CCA Secure\n";



}
