#pragma once

#include "context.h"
#include "matrix.h"

#define ACTIVATION 0
#define NO_ACTIVATION 1

class Evaluator
{
    public:
        Evaluator(std::shared_ptr<Context> context);
        void compress(Matrix compression, const Matrix ciphertext, const Matrix inp, int start = 0, int end = -1);
        //void evaluate(Matrix &dest, const Matrix &compression, const Matrix &cmt, const mpz_t &sfk, int activation = NO_ACTIVATION, int start = 0, int end = -1);

    private:
        std::shared_ptr<Context> ctx;
        Evaluator() = delete;
        static void compress_util_IP(Evaluator &eval, Matrix compression, const Matrix ciphertext, const Matrix inp, int start, int end, int tid, int numThreads);
        //static void evaluate_util_IP(Evaluator &eval, Matrix &dest, const Matrix &compression, const Matrix &cmt, const mpz_t &sfk, int activation, int start, int end, int tid, int numThreads);
};