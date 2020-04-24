#include "Enclave.h"
#include "enclave_t.h"
#include <math.h>
#include <vector>
//#include <gmpxx.h>
//#include <mutex>
#include "Queue.h"
#include "../tools/sgx_tgmp.h"
#include "../tools/gmpxx.h"
#include <mutex>
#include <map>
#include <condition_variable>

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

//std::mutex gaurd;
static std::map<mpz_t, mpz_t> lookup;

// Compute the sigmoid and round to nearest integer
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

// Straight forward lookup in lookup table to get discrete log value
int get_discrete_log(mpz_t &x, mpz_t p)
{
    std::map<mpz_t, mpz_t, MapComp>::iterator it = lookup.find(x);
    if(it == lookup.end())
    {
        mpz_invert(x, x, p);
        it = lookup.find(x);
        if(it == lookup.end())
            mpz_set_si(x, -1);
        else
            mpz_mul_si(x, it->second, -1);
    }
    else
    {
        mpz_set(x, it->second);
    }
    return COMPLETED;
}

/** Utility function to generate lookup table
 * Currently supports only num_threads = 1
 */
int compute_lookup_table(Request &req)
{
    // Temporary variable
    mpz_t tmp;
    mpz_init(tmp);

    mpz_t limit_;
    mpz_init(limit_);
    mpz_ui_pow_ui(limit_, 2, req->limit);

    mpz_t i;
    mpz_init(i);
    mpz_set_si(i, req->tid);

    mpz_t p;
    mpz_init(p);
    mpz_t g;
    mpz_init(g);
    mpz_set_str(p, req->p, BASE);
    mpz_set_str(g, req->g, BASE);

    while(mpz_cmp(limit_, i) >= 0)
    {
        mpz_powm(tmp, g, i, p);
        //std::unique_lock<std::mutex> locker(gaurd);
        mpz_set(lookup[tmp], i);
        mpz_invert(tmp, tmp, p);
        mpz_set_si(lookup[tmp], -mpz_get_si(i));
        //locker.unlock();
        mpz_add_ui(i, i, 1);
    }
    mpz_clear(tmp);
    mpz_clear(p);
    mpz_clear(g);
    mpz_clear(limit_);
    return COMPLETED;
}

// // Straight forward lookup in lookup table
// void ecall_get_discrete_log(mpz_t &x, std::shared_ptr<Context> ctx)
// {
//     mpz_class key{x};
//     std::map<mpz_class, mpz_class, MapComp>::iterator it = lookup.find(key);
//     if(it == lookup.end())
//     {
//         mpz_invert(x, x, ctx->p);
//         mpz_class key{x};
//         it = lookup.find(key);
//         if(it == lookup.end())
//             mpz_set_si(x, -1);
//         else
//             mpz_mul_si(x, it->second.get_mpz_t(), -1);
//     }
//     else
//     {
//         mpz_set(x, it->second.get_mpz_t());
//     }
// }

// // Utility function to generate lookup table
// void ecall_lookup_table_util(std::shared_ptr<Context> ctx, mpz_class limit, int tid, int numThreads)
// {
//     // Temporary variable
//     mpz_t tmp;
//     mpz_init(tmp);

//     mpz_class i = tid;
//     while(i < std::min(limit, mpz_class{ctx->p}))
//     {
//         mpz_powm(tmp, ctx->g, i.get_mpz_t(), ctx->p);
//         std::unique_lock<std::mutex> locker(gaurd);
//         lookup[mpz_class{tmp}] = i;
//         mpz_invert(tmp, tmp, ctx->p);
//         lookup[mpz_class{tmp}] = -i;
//         locker.unlock();
//         i = i + numThreads;
//     }
//     mpz_clear(tmp);
// }

// // Precompute the lookup table
// void ecall_compute_lookup_table(std::shared_ptr<Context> ctx, int bound)
// {
//     if(ctx == NULL)
//         throw std::invalid_argument("Context cannot be NULL!");

//     mpz_t tmp;
//     mpz_init(tmp);

//     mpz_class i = 0;
//     mpz_class new_m = pow(2, bound);

//     // Define threadpool
//     int numThreads = std::thread::hardware_concurrency();
//     std::vector<std::thread> threads(numThreads);
//     for(int i = 0; i < numThreads; i++)
//     {
//         threads[i] = std::thread(ecall_lookup_table_util, ctx, new_m, i, numThreads);
//     }
//     for(int i = 0; i < numThreads; i++)
//         threads[i].join();
//     //std::cout << "Size of lookup table: " << lookup.size() << std::endl;
//     mpz_clear(tmp);
// }

// // Utility function for updating weights
// void ecall_update_weights_util(Logistic_Regression &log_reg, Matrix &update, float alpha, float lr, mpz_class mod, int tid, int numThreads)
// {
//     int col = tid;
//     mpz_t x;
//     mpz_init(x);
//     try
//     {
//         while(col < log_reg.weights.cols)
//         {
//             mpf_class tmp = ((mpz_get_si(mat_element2(log_reg.weights, 0, col))*alpha) + mpz_get_si(mat_element2(update, 0 ,col)))*lr;
//             mpz_set_si(x, 0);
//             if(mpf_cmp_si(tmp.get_mpf_t(), 0.0) > 0)
//             {
//                 if(mpf_cmp_si(tmp.get_mpf_t(), tmp.get_d()) > 0)
//                     mpz_set_si(x, tmp.get_d() + 1);
//                 else
//                     mpz_set_si(x, tmp.get_d());
//             }
//             else if(mpf_cmp_si(tmp.get_mpf_t(), 0.0) < 0)
//             {
//                 if(mpf_cmp_si(tmp.get_mpf_t(), tmp.get_d()) < 0)
//                     mpz_set_si(x, tmp.get_d() - 1);
//                 else
//                     mpz_set_si(x, tmp.get_d());
//             }
//             mpz_add(mat_element2(log_reg.weights, 0, col), mat_element2(log_reg.weights, 0, col), x);
//             col = col + numThreads;
//         }
//     }
//     catch(int e)
//     {
//         std::cout << "Something very wrong has occured.\n";
//         exit(EXIT_FAILURE);
//     }
// }

// void ecall_update_weights(Logistic_Regression &mdl, Matrix &update_compress, Matrix &cmt, Evaluator &eval, Matrix &training_error,
//                 float alpha, float learning_rate, int start_idx, int batch_size, int update_r, int update_c)
// {
//     // Initialize sfk_update
//     mpz_t sfk_update;
//     mpz_init(sfk_update);

//     Matrix update(update_r, update_c);
//     Matrix update_trans(update.cols, update.rows);
    
//     // Compute update to be made
//     Matrix::row_inner_product(sfk_update, sk_2->data(), training_error, -1, 0, 0, start_idx, start_idx + batch_size - 1);
//     eval.evaluate(update, update_compress, cmt, sfk_update, NO_ACTIVATION);
//     update.transpose(update_trans);
//     mpz_class mod{mdl.ctx->p};

//     // Define threadpool
//     int numThreads = mdl.weights.cols;
//     int numCores = std::thread::hardware_concurrency();
//     if(numThreads > numCores)
//         numThreads = numCores;
    
//     std::vector<std::thread> threads(numThreads);
//     for(int i = 0; i < numThreads; i++)
//     {
//         threads[i] = std::thread(ecall_update_weights_util, std::ref(mdl), std::ref(update_trans), alpha, learning_rate, mod, i, numThreads);
//     }
//     for(int i = 0; i < numThreads; i++)
//         threads[i].join();

//     mpz_clear(sfk_update);
// }

// void ecall_enclave_prediction(Logistic_Regression &mdl, Matrix &ypred, Matrix &compression, Matrix &cmt, Evaluator &eval)
// {
//     Matrix::row_inner_product(mdl.sfk, mdl.weights, sk_1->data());
//     eval.evaluate(ypred, compression, cmt, mdl.sfk, ACTIVATION);
// }

// void ecall_set_secret_key(std::shared_ptr<Secret_Key> sk, int id)
// {
//     if(id == 1)
//         sk_1 = std::make_unique<Secret_Key>(sk);
//     else if(id == 2)
//         sk_2 = std::make_unique<Secret_Key>(sk);
// }


/**
 * Decrypt commitments and compute the FE result
 * Currently commitments are not encfypted
 * Supports num_threads = 1
 */
int evaluate(Matrix &dest, const Matrix &compression, const Matrix &cmt, const mpz_t &sfk, int activation, Request &req, int start, int end, int mode)
{
    if(dest == NULL || compression == NULL || cmt == NULL || sfk == NULL)
        return ERROR;
    int row = req->tid;
    mpz_t ct0;
    mpz_init(ct0);

    mpz_t tmp;
    mpz_init(tmp);

    mpz_t p;
    mpz_init(p);
    mpz_t g;
    mpz_init(g);
    mpz_set_str(p, req->p, BASE);
    mpz_set_str(g, req->g, BASE);

    if(end == -1)
        end = dest->rows - 1;

    while(row < end + 1)
    {
        mpz_set(ct0, mat_element(cmt, row, 0));
        mpz_powm(ct0, ct0, sfk, p);
        mpz_invert(ct0, ct0, p);
        mpz_set_si(tmp, 1);

        mpz_mul(tmp, mat_element(compression, row, 0), ct0);
        mpz_mod(tmp, tmp, p);
        get_discrete_log(tmp,p);
        if(activation == ACTIVATION)
            sigmoid(tmp, mpz_get_d(tmp));

        if(mode == ENCRYPT)
        {
            // Encrypt tmp using enclave pk
        }
        else
            mpz_set(mat_element(dest, 0, row), tmp);
        row = row + req->num_threads;
    }
    mpz_clear(tmp);
    mpz_clear(ct0);
    mpz_clear(p);
    mpz_clear(g);
    return COMPLETED;
}

/**
 * Function for updating the weights
 * input_1 -> null
 * output-> weights
 */
int update_weights(Request &req)
{
    // Initialize sfk_update
    mpz_t sfk_update;
    mpz_init(sfk_update);

    Matrix update = mat_init(req->wp->update_rows, req->wp->update_cols);
    Matrix update_trans = mat_init(req->input_1->cols, req->input_1->rows);
    
    // Compute update to be made
    row_inner_product(sfk_update, sk_2->data(), req->wp->training_error, -1, 0, 0, req->wp->start_idx, req->wp->start_idx + req->wp->batch_size - 1);
    evaluate(update, req->compression, req->cmt, sfk_update, NO_ACTIVATION, req, 0, -1, NO_ENCRYPT);
    transpose(update_trans, update);

    int col = req->tid;
    mpz_t x;
    mpz_init(x);

    mpz_t wt_tmp;
    mpz_init(wt_tmp);

    // mpf_t tmp;
    // mpf_init(tmp);
    mpf_class tmp;

    try
    {
        while(col < req->output->cols)
        {
            mpz_set(wt_tmp, mat_element(req->output, 0, col));
            mpf_class tmp = ((mpz_get_si(wt_tmp)* req->wp->alpha) + mpz_get_si(mat_element(update_trans, 0 ,col)))*req->wp->learning_rate;
            //mpf_set(tmp, ((mpz_get_si(wt_tmp)* req->wp->alpha) + mpz_get_si(mat_element(update_trans, 0 ,col)))*req->wp->learning_rate);
            mpz_set_si(x, 0);
            if(mpf_cmp_si(tmp.get_mpf_t(), 0.0))
            {
                if(mpf_cmp_si(tmp.get_mpf_t(), tmp.get_d() > 0))
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
            mpz_add(mat_element(req->output, 0, col), wt_tmp, x);
            col = col + req->num_threads;
        }
    }
    catch(int e)
    {
        return ERROR;
    }

    mpz_clear(x);
    mpz_clear(wt_tmp);
    delete_matrix(update);
    delete_matrix(update_trans);
    return COMPLETED;
}

// input_1 -> null
// output -> ypred
int predict_final(Request &req)
{
    // DECRYPT CMT!!
    if(req->output == NULL || req->compression == NULL || req->cmt == NULL)
        return ERROR;

    mpz_t sfk;
    mpz_init(sfk);
    mpz_set_str(sfk, req->final_sfk, BASE);

    int result = evaluate(req->output, req->compression, req->cmt, sfk, ACTIVATION, req, 0, -1, ENCRYPT);

    // Free mpz_t
    mpz_clear(sfk);
    return result;
}

// input_1 -> weights
// output ypred
int predict_train(Request &req)
{
    mpz_t sfk;
    mpz_init(sfk);

    if(req->output == NULL || req->compression == NULL || req->cmt == NULL || req->wp == NULL || req->input_1 == NULL)
        return ERROR;
    row_inner_product(sfk, req->input_1, sk_1->data());

    // DECRYPT CMT!!
    int result = evaluate(req->output, req->compression, req->cmt, sfk, ACTIVATION, req, 0, -1, NO_ENCRYPT);

    mpz_clear(sfk);
    return result;
}

/**
 * Setup the secret keys required to compute the function secret
 * This method is only for testing
 * In a practical scenario, the client is expected to encrypt these values and send them to the enclave
 */
int setup_secret_key(Request &req)
{
    // Input_1 -> data for secret key
    if(req->input_1 == NULL || req->key_id < 1 || req->key_id > 2)
        return ERROR;
    if(req->key_id == 1)
        sk_1 = new Secret_Key(req->input_1);
    else
        sk_2 = new Secret_Key(req->input_1);
    return COMPLETED;
}

/**
 * Setup the secret function key
 * @params : req (input_1 = weights)
 * @return : store result in req->final_sfk
 */
int set_sfk(Request &req)
{
    if(req->input_1 == NULL)
        return ERROR;
    mpz_t sfk;
    mpz_init(sfk);
    row_inner_product(sfk, req->input_1, sk_1->data());
    mpz_get_str(req->final_sfk, BASE, sfk);
    return COMPLETED;
}

/**
 * Listen for requests from application and process them
 * @params : queue containing requests to be serviced
 * @return : int
 */
int enclave_service(void* arg)
{
    if(arg == NULL)
        return FAILURE;

    // Define task queue
    Queue* task_queue = (Queue*)arg;

    // Start listening for request
    while(true)
    {
        Request req = task_queue->dequeue();
        req->status = SCHEDULED;
        if(req == NULL)
            return ERROR;

        switch(req->job_id)
        {
            case GENERATE_LOOKUP_TABLE:
            {
                req->status = compute_lookup_table(req);
                break;
            }
            case FINAL_PREDICTION:
            {
                req->status = predict_final(req);
                break;
            }
            case TRAIN_PREDICTION:
            {
                req->status = predict_train(req);
                break;
            }
            case WEIGHT_UPDATE:
            {
                req->status = update_weights(req);
                break;
            }
            case SET_FE_SECRET_KEY:
            {
                req->status = setup_secret_key(req);
                break;
            }
            case SET_SFK:
            {
                req->status = set_sfk(req);
                break;
            }
            case GET_PUB_KEY:
                break;
        }
    }
    return SUCCESS;
}
