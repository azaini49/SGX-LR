#include "evaluator.h"
#include <cmath>
#include <thread>
#include <mutex>
#include "helper.h"
#include <iostream>

Evaluator::Evaluator(std::shared_ptr<Context> context)
    :ctx(context)
{
    if(ctx == NULL)
        throw std::invalid_argument("Context cannot be null!");
}

void Evaluator::compress_util_IP(Evaluator &eval, Matrix &compression, const Matrix &ciphertext, const Matrix &inp, int start, int end, int tid, int numThreads)
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


void Evaluator::compress(Matrix &compression, const Matrix &ciphertext, const Matrix &inp, int start, int end) 
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

// void Evaluator::evaluate_util_IP(Evaluator &eval, Matrix &dest, const Matrix &compression, const Matrix &cmt, const mpz_t &sfk, int activation, int start, int end, int tid, int numThreads)
// {
//     int row = tid;
//     mpz_t ct0;
//     mpz_init(ct0);

//     while(row < end + 1)
//     {
//         mpz_set(ct0, mat_element2(cmt, row, 0));
//         mpz_powm(ct0, ct0, sfk, eval.ctx->p);
//         mpz_invert(ct0, ct0, eval.ctx->p );
//         mpz_set_si(mat_element2(dest, row, 0), 1);

//         mpz_mul(mat_element2(dest, row, 0), mat_element2(compression, row, 0), ct0);
//         mpz_mod(mat_element2(dest, row, 0), mat_element2(dest, row, 0), eval.ctx->p);
//         ecall_get_discrete_log(mat_element2(dest, row, 0), eval.ctx);
//         if(activation == ACTIVATION)
//             sigmoid(mat_element2(dest, row, 0), mpz_get_d(mat_element2(dest, row, 0)));
//         row = row + numThreads;
//     }
//     mpz_clear(ct0);
// }

// void Evaluator::evaluate(Matrix &dest, const Matrix &compression, const Matrix &cmt, const mpz_t &sfk, int activation, int start, int end)
// {
//     if(compression.rows != dest.rows || compression.rows != cmt.rows || cmt.cols != dest.cols || cmt.cols != compression.cols || cmt.cols != 1)
//     {
//         std::cout << "Compression(" << compression.rows << ", " << compression.cols << ")\n";
//         std::cout << "Destination(" << dest.rows << ", " << dest.cols << ")\n";
//         std::cout << "Commitment(" << cmt.rows << ", " << cmt.cols << ")\n";
//         throw std::invalid_argument("Invalid dimensions!!");
//     }

//     if(end == -1)
//         end = dest.rows - 1;

//     // Define threadpool
//     int numThreads = end - start + 1;
//     int numCores = std::thread::hardware_concurrency();
//     if(numThreads > numCores)
//         numThreads = numCores;

//     std::vector<std::thread> threads(numThreads);
//     for(int i = start; i < start + numThreads; i++)
//     {
//         threads[i-start] = std::thread(evaluate_util_IP, std::ref(*this), std::ref(dest), std::ref(compression), std::ref(cmt), std::ref(sfk), activation, start, end, i, numThreads);
//     }
//     for(int i = 0; i < numThreads; i++)
//         threads[i].join();
// }

