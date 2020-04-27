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
void Evaluator::compress_util_IP(Evaluator &eval, Matrix compression, const Matrix ciphertext, const Matrix inp, int start, int end, int tid, int numThreads)
{
    int row = tid;

    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);

    while(row < end + 1)
    {
        mpz_set_si(mat_element(compression, row, 0), 1);
        for(int col = 0; col < ciphertext->cols; col++)
        {
            mpz_powm(tmp, mat_element(ciphertext, row, col), mat_element(inp, 0, col), eval.ctx->p);
            mpz_mul(mat_element(compression, row, 0), mat_element(compression, row, 0), tmp);
            mpz_mod(mat_element(compression, row, 0), mat_element(compression, row, 0), eval.ctx->p);
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
