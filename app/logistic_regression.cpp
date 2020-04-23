#include "logistic_regression.h"
#include "helper.h"
#include <thread>
#include "../enclave/Enclave.h"
#include "sgx_urts.h"
#include "../include/shared.h"
#include <iostream>

#define TRAIN 0
#define PREDICTION 1

Logistic_Regression::Logistic_Regression(std::shared_ptr<Context> context, int iter, float reg_const)
    :ctx(context), iterations(iter), lambda(reg_const)
{
    mpz_init(this->sfk);
}

Logistic_Regression::~Logistic_Regression()
{
    mpz_clear(this->sfk);
    delete_matrix(this->weights);
}

void Logistic_Regression::predict(Matrix &ypred, Matrix &xtest_enc, Matrix &cmt, Evaluator &eval)
{
    Matrix compression = mat_init(xtest_enc->rows, 1);
    eval.compress(compression, xtest_enc, this->weights);
    
    Request req = init_request(FINAL_PREDICTION, 1);
    mpz_set(req->final_sfk, this->sfk);
    req->output = ypred;
    req->cmt = cmt;
    req->compression = compression;
    make_request(req);
    delete_matrix(compression);
}

void Logistic_Regression::enclave_set_sfk()
{
    row_inner_product(this->sfk, this->weights, sk_1->data());
}

void Logistic_Regression::compute_performance_metrics(const Matrix &ypred, const Matrix &ytrue)
{
    if(ypred->cols != ytrue->cols || ypred->rows != ytrue->rows)
    {
        std::cout << "[CPM] Ypred [" << ypred->rows << ", " << ypred->cols << "]\n";
        std::cout << "[CPM] Ytrue [" << ytrue->rows << ", " << ytrue->cols << "]\n";
        throw std::invalid_argument("Invalid Dimensions!");
    }
    Matrix diff = mat_init(ypred->rows, ypred->cols);
    compute_hamming_distance(diff, ypred, ytrue);

    int k = 0;
    for(int i = 0; i < diff->cols; i++)
        k = k + mpz_get_si(mat_element(diff, 0, i));
    this->accuracy = 1 - ((float)k/diff->cols);

}

void Logistic_Regression::train(Matrix &xtrain_enc, Matrix &xtrain_trans_enc, Matrix &ytrain, Matrix &cmt_xtrain, Matrix &cmt_xtrain_trans, int batchSize, float learning_rate)
{
    // Check input dimensions (ytrain includes an extra col for the commitment)
    if(xtrain_enc->rows != ytrain->cols || ytrain->rows != 1)
    {
        std::cout << "EXPECTED: xtrain(" << xtrain_enc->rows << " x " << xtrain_enc->cols << ") | ytrain(1 x " << ytrain->cols << ")\n";
        std::cout << "RECEIVED: xtrain(" << xtrain_enc->rows << ", " << xtrain_enc->cols << ") | ytrain( " << ytrain->rows << ", " << ytrain->cols << ")\n";
        throw std::invalid_argument("[LOG_REG] Dimensions of training input are incompatible!");
    }

    // Store intermediate predictions in ypred
    Matrix ypred = mat_init(batchSize, ytrain->rows);
    Matrix ypred_trans = mat_init(ypred->cols, ypred->rows);

    // Store intermediate compression
    Matrix compression = mat_init(batchSize, ytrain->rows);

    // Initialize the weights to a zero vector
    this->weights = mat_init(1, xtrain_enc->cols);

    // Initialize L2 regularization constant as alpha
    float alpha = 1.0/this->lambda;

    // Store the xtrain_enc batch
    Matrix xtrain_batch = mat_init(batchSize, xtrain_enc->cols);
    Matrix xtrain_batch_cmt = mat_init(batchSize, cmt_xtrain->cols);
    Matrix xtrain_trans_batch_cmt = mat_init(batchSize, cmt_xtrain_trans->cols);

    // Store xbatch transpose and training error
    Matrix xbatch = mat_init(xtrain_trans_enc->rows, batchSize); // batch from xtrain_trans enc
    Matrix training_error = mat_init(ypred_trans->rows, ypred_trans->cols); // training error for batch

    // Update to be made to the weight vector
    Matrix update_compress = mat_init(xbatch->rows, training_error->rows);

    // Batch starts from start_idx
    int start_idx = 0;

    // Create evaluators
    Evaluator eval(this->ctx);

    // Setup request object to communicate with the enclave
    Request train_req = init_request(TRAIN_PREDICTION, 1);
    train_req->wp = init_wrapper(alpha, learning_rate);
    train_req->wp->batch_size = batchSize;
    train_req->wp->update_rows = xbatch->rows;
    train_req->wp->update_cols = training_error->rows;
    mpz_set(train_req->p, this->ctx->p);
    mpz_set(train_req->g, this->ctx->g);

    // Train for specified number of iterations
    for(int step = 0; step < this->iterations; step++)
    {
        std::cout << "Iteration : " << step << std::endl;

        // Obtain the batch to train over
        mat_splice(xtrain_batch, xtrain_enc, start_idx, start_idx + batchSize - 1, 0, xtrain_enc->cols - 1);
        mat_splice(xtrain_batch_cmt, cmt_xtrain, start_idx, start_idx + batchSize - 1, 0, cmt_xtrain->cols - 1);

        // Compress the batch input
        eval.compress(compression, xtrain_batch, this->weights);

        // Assign values to request and wrapper object
        train_req->wp->start_idx = start_idx;
        train_req->input_1 = this->weights;
        train_req->output = ypred;
        train_req->compression = compression;
        train_req->cmt = xtrain_batch_cmt;

        // The enclave computes computes the prediction
        make_request(train_req);
        //ecall_enclave_prediction(std::ref(*this), ypred, compression, xtrain_batch_cmt, eval);
        transpose(ypred_trans, ypred);
        
        // Compute training error
        compute_vector_difference(training_error, ytrain, ypred_trans, 0, start_idx, start_idx + batchSize - 1);

        // Get the xbatch_trans matrix
        mat_splice(xbatch, xtrain_trans_enc, 0, xtrain_trans_enc->rows - 1, start_idx, start_idx + batchSize - 1);

        eval.compress(update_compress, xbatch, training_error);

        // Assign values to request and wrapper for updating weights
        train_req->output = this->weights;
        train_req->compression = update_compress;
        train_req->cmt = cmt_xtrain_trans;
        train_req->wp->training_error = training_error;
        make_request(train_req);

        //ecall_update_weights(std::ref(*this), update_compress, cmt_xtrain_trans, eval, training_error, alpha, learning_rate, start_idx, batchSize, xbatch->rows, training_error->rows);
        start_idx = (start_idx + batchSize) % xtrain_enc->rows;
        if(start_idx + batchSize >= xtrain_enc->rows)
            start_idx = xtrain_enc->rows - batchSize;
    }
    enclave_set_sfk();

    // Delete local matrices
    delete_matrix(ypred);
    delete_matrix(ypred_trans);
    delete_matrix(compression);
    delete_matrix(xtrain_batch);
    delete_matrix(xtrain_batch_cmt);
    delete_matrix(xtrain_trans_batch_cmt);
    delete_matrix(xbatch);
    delete_matrix(training_error);
    delete_matrix(update_compress);
}