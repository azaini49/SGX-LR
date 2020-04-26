#pragma once

#include "context.h"
#include "evaluator.h"
#include "matrix.h"

class Logistic_Regression
{
    public:
        std::shared_ptr<Context> ctx;
        int iterations;
        float lambda;
        float accuracy;
        Matrix weights;
        mpz_t sfk;
        Logistic_Regression(std::shared_ptr<Context> context, int iter = 10, float reg_const = 5.0);
        ~Logistic_Regression();
        void train(Matrix xtrain_enc, Matrix xtrain_trans_enc, Matrix ytrain, Matrix cmt, Matrix cmt_xtrain_trans, int batchSize = 1024, float learningRate = 0.004);
        void predict(Matrix ypred, Matrix xtest_enc, Matrix cmt, Evaluator &eval);
        void compute_performance_metrics(const Matrix ypred, const Matrix ytrue);
        
    private:
        void enclave_set_sfk();
};