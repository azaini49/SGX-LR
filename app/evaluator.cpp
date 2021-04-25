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
         std::cout << "pre compression row: "<< row <<", col:" << 0 << ", val: "<< mpz_get_si(mat_element(compression, row, 0)) <<"\n";
         for(int col = 0; col < ciphertext->cols; col++)
         {

             mpz_powm(tmp, mat_element(ciphertext, row, col), mat_element(inp, 0, col), eval.ctx->Ns); //take cti^yi
             mpz_mul(mat_element(compression, row, 0), mat_element(compression, row, 0), tmp); // mult cti^yi times prev
             mpz_mod(mat_element(compression, row, 0), mat_element(compression, row, 0), eval.ctx->Ns); // take mod of above
             std::cout << "pre compression row: "<< row <<", col:" << col << ", val: "<< mpz_get_si(mat_element(compression, row, 0)) <<"\n";
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
}

//TODO! update
void Evaluator::evaluate_util_IP(Evaluator &eval, Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t &sfk, int activation, int start, int end, int tid, int numThreads)
{
    int row = tid;
    mpz_t ct0;
    mpz_init(ct0);

    mpz_t one;
    mpz_init_set_si(one, 1);

    mpz_t N_inv;
    mpz_init(N_inv);

    while(row < end + 1)
    {
        std::cout << "compressed vector for thread " << tid << ": "<< mpz_get_si(mat_element(compression, row, 0)) <<"\n";
        mpz_set(ct0, mat_element(cmt, row, 0));
        mpz_pow_ui(ct0, ct0, sfk); // take ct0^sfk
        mpz_invert(ct0, ct0, eval.ctx->Ns); // find 1/ct0^sfk
        mpz_set_si(mat_element(dest, row, 0), 1);

        mpz_mul(mat_element(dest, row, 0), mat_element(compression, row, 0), ct0); // find compression / ct0^sfk
        mpz_mod(mat_element(dest, row, 0), mat_element(dest, row, 0), eval.ctx->Ns); // take mod of prev value

        mpz_sub(mat_element(dest, row, 0), one); // subtract 1 from compression / ct0^sfk
        mpz_invert(N_inv, eval.ctx->N, eval.ctx->Ns); // invert N
        mpz_mul(mat_element(dest, row, 0), mat_element(dest, row, 0), N_inv); // divide compression / ct0^sfk -1  by N
        mpz_mod(mat_element(dest, row, 0), mat_element(dest, row, 0), eval.ctx->Ns); // take mod of prev value

        //get_discrete_log(mat_element(dest, row, 0), eval.ctx);
        if(activation == ACTIVATION)
            sigmoid(mat_element(dest, row, 0), mpz_get_d(mat_element(dest, row, 0)));
        row = row + numThreads;
    }
    mpz_clear(ct0);
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
}
