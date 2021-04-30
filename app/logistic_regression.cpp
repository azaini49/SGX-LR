#include "logistic_regression.h"
#include "helper.h"
#include <thread>
#include "sgx_urts.h"
#include "../include/shared.h"
#include <iostream>
#include "../tools/secret_key.h"

#define TRAIN 0
#define PREDICTION 1



int evaluate_e(E_Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t sfk,  mpz_t N, mpz_t Ns, int start, int end)
{
    if(dest == NULL || compression == NULL || cmt == NULL)
        return ERROR;
    int row = 0;
    mpz_t ct0;
    mpz_init(ct0);
    mpz_t tmp;
    mpz_init(tmp);

    if(end == -1)
        end = dest->rows - 1;

    while(row < end + 1)
    {
        mpz_set(ct0, mat_element(cmt, row, 0));
        mpz_powm(ct0, ct0, sfk, Ns);
        mpz_invert(ct0, ct0, Ns);
        mpz_set_si(tmp, 1);

        mpz_mul(tmp, mat_element(compression, row, 0), ct0);
        mpz_mod(tmp, tmp, Ns);

        //get_discrete_log(tmp, res->N);
        mpz_sub_ui(tmp, tmp, 1); // subtract 1 from compression / ct0^sfk
        mpz_tdiv_q(tmp, tmp, N); // divide compression / ct0^sfk -1  by N
        mpz_mod(tmp, tmp, Ns); // take mod of prev value

        mpz_set(mat_element(dest, row, 0), tmp);
        if (row == 0) std::cout << "in UPDATE WEIGHTS " << mpz_get_si(tmp) << "\n";


	row = row + 1;
    }
    mpz_clear(tmp);
    mpz_clear(ct0);
    return COMPLETED;
}



/**
 * Function for updating the weights
 * input_1 -> training_error
 * output-> weights
 */
int update_weights(Secret_Key sk_2, mpz_t N, mpz_t Ns, Matrix input, Matrix output, Matrix compression, Matrix cmt, int start_idx, int batch_size, int alpha, int learning_rate)
{
    // Initialize sfk_update
    mpz_t sfk_update;
    mpz_init(sfk_update);
    struct e_matrix update;
    struct e_matrix update_trans;
    setup_matrix(&update, output->cols, output->rows);

    // Compute update to be made
    int stat = row_inner_product(sfk_update, sk_2.data(), input, -1, 0, 0, start_idx, start_idx + batch_size - 1);
    if(stat == ERROR)
    {
        return ERROR;
    }

    evaluate_e(&update, compression, cmt, sfk_update, N, Ns, 0, -1);


    int col = 0;
    mpz_t x;
    mpz_init(x);
    mpz_t wt;
    mpz_init(wt);

    try
    {
        while(col < output->cols)
        {
            mpz_set(wt, mat_element(output, 0, col));
            mpf_class tmp = ((mpz_get_si(wt)*alpha) + mpz_get_si(mat_element2(update, 0 ,col)))*learning_rate;
            mpz_set_si(x, 0);
            if(mpf_cmp_si(tmp.get_mpf_t(), 0.0) > 0)
            {
                if(mpf_cmp_si(tmp.get_mpf_t(), tmp.get_d()) > 0)
                    mpz_set_si(x, tmp.get_d() + 1);
                else
                    mpz_set_si(x, tmp.get_d());
            }
            else if(mpf_cmp_si(tmp.get_mpf_t(), 0.0) < 0)
            {
                if(mpf_cmp_si(tmp.get_mpf_t(), tmp.get_d()) < 0)
                    mpz_set_si(x, tmp.get_d() - 1);
                else
                    mpz_set_si(x, tmp.get_d());
            }
            mpz_add(wt, wt, x);
            mpz_set(mat_element(output, 0, col), wt);
            col = col + 1;
        }
    }
    catch(int e)
    {
        return ERROR;
    }

    mpz_clear(x);
    mpz_clear(wt);
    mpz_clear(sfk_update);
    return COMPLETED;
}


/**
 * Constructor for Logistic_Regression
 * @params : context, number of training iteration, regularization constant
 */
Logistic_Regression::Logistic_Regression(std::shared_ptr<Context> context, int iter, float reg_const)
    :ctx(context), iterations(iter), lambda(reg_const)
{}

/**
 * Default destructor
 */
Logistic_Regression::~Logistic_Regression()
{
    delete_matrix(this->weights);
}

/**
 * Make prediction given the encrypted input and commitment
 * @params : encrypted input, encrypted cmt, evaluator
 * @return : store result in ypred
 */
void Logistic_Regression::predict(Matrix ypred, Matrix xtest_enc, Matrix cmt, Evaluator &eval, Public_Key pk)
{
    Timer timer("Prediction Time");
    Matrix dummy = NULL;
    Matrix compression = mat_init(xtest_enc->rows, 1);
    eval.compress(compression, xtest_enc, this->weights);
    Request req = serialize_request(FINAL_PREDICTION, pk.data(), ypred, compression, cmt, mpz_class{ctx->N}, mpz_class{ctx->Ns}, mpz_class{ctx->g});
    make_request(req);
    //delete_matrix(compression);
}

/**
 * Compute accuracy
 * TODO : Compute precision, recall, f1 score
 */
void Logistic_Regression::compute_performance_metrics(const Matrix ypred, const Matrix ytrue)
{
    if(ypred->cols != ytrue->cols || ypred->rows != ytrue->rows)
    {
        std::cout << "[CPM] Ypred [" << ypred->rows << ", " << ypred->cols << "]\n";
        std::cout << "[CPM] Ytrue [" << ytrue->rows << ", " << ytrue->cols << "]\n";
        throw std::invalid_argument("Invalid Dimensions!");
    }
    Matrix diff = mat_init(ypred->rows, ypred->cols);
    //compute_hamming_distance(diff, ypred, ytrue);
    //compute_vector_difference(diff, ypred, ytrue);
    compute_stat_distance(diff, ypred, ytrue);
    int tn = 0;
    int tp = 0;
    int fn = 0;
    int fp = 0;
    int k = 0;
    for(int i = 0; i < diff->cols; i++){
      k = mpz_get_si(mat_element(diff, 0, i));
      if(k == 0) tn = tn + 1.0;
      else if(k == 1) tp = tp + 1.0;
      else if(k == 2) fn = fn + 1.0;
      else if(k == 3) fp = fp + 1.0;
    }
    this->tn = tn;
    this->tp = tp;
    this->fn = fn;
    this->fp = fp;
    this->accuracy = (1.0*tp + tn)/(tp + tn + fp + fn);
    this->precision = (1.0*tp)/(tp + fp);
    this->recall = (1.0*tp)/(tp + fn);
    this->f1 = 2.0*(this->precision*this->recall)/(this->precision+ this->recall);
    //int k = 0;
    //for(int i = 0; i < diff->cols; i++)
    //    k = k + mpz_get_si(mat_element(diff, 0, i));
    //this->accuracy = 1 - ((float)k/diff->cols);

}

/**
 * Make a request to the enclave to set the function key
 */
void Logistic_Regression::enclave_set_sfk()
{
    Matrix dummy = NULL;
    Request req = serialize_request(SET_SFK, this->weights, dummy, dummy, dummy, 0, 0, 0, 0);
    make_request(req);
}

/**
 * Train the model in given inputs to obtain model parameters
 */
void Logistic_Regression::train(Matrix xtrain_enc, Matrix xtrain_trans_enc, Matrix ytrain, Matrix cmt_xtrain, Matrix cmt_xtrain_trans, int batchSize, float learning_rate, Matrix sk_2_data)
{
    Timer timer("Training Time");
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
    make_zero(this->weights);

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
    Request train_req;
    Matrix dummy = NULL;

    // Train for specified number of iterations
    for(int step = 0; step < this->iterations; step++)
    {
        //std::cout << "################################### Iteration : " << step << std::endl;
        //make_zero(update_compress);
	//make_zero(weights_temp);


        // Obtain the batch to train over
        mat_splice(xtrain_batch, xtrain_enc, start_idx, start_idx + batchSize - 1, 0, xtrain_enc->cols - 1);
        mat_splice(xtrain_batch_cmt, cmt_xtrain, start_idx, start_idx + batchSize - 1, 0, cmt_xtrain->cols - 1);

	std::cout << "enc x " << mpz_get_si(mat_element(xtrain_batch, 0,0)) << "\n";
        std::cout << "enc x " << mpz_get_si(mat_element(xtrain_batch, 0,1)) << "\n";

        // Compress the batch input
        eval.compress(compression, xtrain_batch, this->weights);

	std::cout << "compression " << mpz_get_si(mat_element(compression, 0,0)) << "\n";
	std::cout << "compression " << mpz_get_si(mat_element(compression, 0,1)) << "\n";

        // Assign values to request object
        train_req = serialize_request(TRAIN_PREDICTION, this->weights, ypred, compression, xtrain_batch_cmt, mpz_class{ctx->N}, mpz_class{ctx->Ns}, mpz_class{ctx->g});
        train_req->start_idx = start_idx;
        train_req->batch_size = batchSize;
        make_request(train_req);
	std::cout << "step " << step << "ypred " << mpz_get_si(mat_element(ypred, 0,0)) << "\n";
        transpose(ypred_trans, ypred);

	    // Compute training error
        compute_vector_difference(training_error, ytrain, ypred_trans, 0, start_idx, start_idx + batchSize - 1);
        std::cout << "step " << step << "train err " << mpz_get_si(mat_element(training_error, 0,0)) << "\n";
        // Get the xbatch_trans matrix
        mat_splice(xbatch, xtrain_trans_enc, 0, xtrain_trans_enc->rows - 1, start_idx, start_idx + batchSize - 1);

        eval.compress(update_compress, xbatch, training_error);
        std::cout << "u compression " << mpz_get_si(mat_element(update_compress, 0,0)) << "\n";
        std::cout << "u compression " << mpz_get_si(mat_element(update_compress, 0,1)) << "\n";


	Secret_Key sk_2;
	sk_2.set_key(sk_2_data);
	Matrix weights_temp = mat_init(this->weights->rows, this->weights->cols);
	update_weights(sk_2, ctx->N, ctx->Ns, training_error, weights_temp, update_compress, cmt_xtrain_trans, start_idx, batchSize, alpha, learning_rate);


        // Assign values to request and wrapper for updating weights
        Request update_weights_req = serialize_request(WEIGHT_UPDATE, training_error, this->weights, update_compress, cmt_xtrain_trans, mpz_class{ctx->N}, mpz_class{ctx->Ns}, mpz_class{ctx->g});
        update_weights_req->start_idx = start_idx;
        update_weights_req->batch_size = batchSize;
        update_weights_req->alpha = alpha;
        update_weights_req->learning_rate = learning_rate;
        make_request(update_weights_req);

        start_idx = (start_idx + batchSize) % xtrain_enc->rows;
        if(start_idx + batchSize >= xtrain_enc->rows)
            start_idx = xtrain_enc->rows - batchSize;
     
        std::cout << "step " << step << "weights " << mpz_get_si(mat_element(this->weights, 0,0)) << "\n";
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
