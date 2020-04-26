#include "Enclave.h"
#include "enclave_t.h"
#include <math.h>
#include <vector>
#include "../include/Queue.h"
#include <inttypes.h>
#include <map>
//#include <condition_variable>

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

//std::mutex gaurd;
static std::map<mpz_class, mpz_class> lookup;

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
    mpz_class key{x};
    std::map<mpz_class, mpz_class, MapComp>::iterator it = lookup.find(key);
    if(it == lookup.end())
    {
        mpz_invert(x, x, p);
        mpz_class key{x};
        it = lookup.find(key);
        if(it == lookup.end())
            mpz_set_si(x, -1);
        else
            mpz_mul_si(x, it->second.get_mpz_t(), -1);
    }
    else
    {
        //printf_enclave("found\n");
        mpz_set(x, it->second.get_mpz_t());
    }
    return COMPLETED;
}

/** Utility function to generate lookup table
 * Currently supports only num_threads = 1
 */
int compute_lookup_table(Response &res)
{
    mpz_t tmp;
    mpz_init(tmp);
    printf_enclave("Limit : %d\n", res->limit);

    mpz_class i = 0;
    mpz_class limit = pow(2, res->limit);
    while(i < limit)
    {
        mpz_powm(tmp, res->g, i.get_mpz_t(), res->p);
        //std::unique_lock<std::mutex> locker(gaurd);
        lookup[mpz_class{tmp}] = i;
        mpz_invert(tmp, tmp, res->p);
        lookup[mpz_class{tmp}] = -i;
        //locker.unlock();
        i = i + 1;
    }
    mpz_clear(tmp);
    return COMPLETED;
}

/**
 * Decrypt commitments and compute the FE result
 * Currently commitments are not encfypted
 * Supports num_threads = 1
 */
int evaluate(Matrix &dest, const Matrix &compression, const Matrix &cmt, const mpz_t &sfk, int activation, Response &res, int start, int end, int mode)
{
    if(dest == NULL || compression == NULL || cmt == NULL || sfk == NULL)
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
        mpz_powm(ct0, ct0, sfk, res->p);
        mpz_invert(ct0, ct0, res->p);
        mpz_set_si(tmp, 1);

        mpz_mul(tmp, mat_element(compression, row, 0), ct0);
        mpz_mod(tmp, tmp, res->p);
        get_discrete_log(tmp, res->p);
        if(activation == ACTIVATION)
            sigmoid(tmp, mpz_get_d(tmp));

        if(mode == ENCRYPT)
        {
            // Encrypt tmp using enclave pk
        }
        else
            mpz_set(mat_element(dest, 0, row), tmp);
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
int update_weights(Response &res)
{
    // printf_enclave("Weights : %d x %d\n", res->output->rows, res->output->cols);
    // printf_enclave("Start : %d   End : %d\n", res->start_idx, res->start_idx + res->batch_size - 1);
    // Initialize sfk_update
    mpz_t sfk_update;
    mpz_init(sfk_update);
    ////printf_enclave("sfk initialized\n");
    Matrix update = mat_init(res->output->cols, res->output->rows);
    //printf_enclave("Update initialized\n");
    Matrix update_trans = mat_init(update->cols, update->rows);

    // printf_enclave("Update : %d x %d\n", update->rows, update->cols);
    // printf_enclave("SK2 : %d x %d\n", sk_2.data()->rows, sk_2.data()->cols);
    
    // Compute update to be made
    row_inner_product(sfk_update, res->input, sk_2.data(), -1, 0, 0, res->start_idx, res->start_idx + res->batch_size - 1);
    evaluate(update, res->compression, res->cmt, sfk_update, NO_ACTIVATION, res, 0, -1, NO_ENCRYPT);
    transpose(update_trans, update);
    print_matrix_e(update_trans);
    //printf_enclave("Update trans : %d x %d\n", update_trans->rows, update_trans->cols);

    int col = 0;
    mpz_t x;
    mpz_init(x);
    mpz_t wt;
    mpz_init(wt);

    try
    {
        while(col < res->output->cols)
        {
            mpz_set(wt, mat_element(res->output, 0, col));
            mpf_class tmp = ((mpz_get_si(wt)*res->alpha) + mpz_get_si(mat_element(update_trans, 0 ,col)))*res->learning_rate;
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
            mpz_set(mat_element(res->output, 0, col), wt);
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
    delete_matrix(update);
    delete_matrix(update_trans);
    return COMPLETED;
}

// input_1 -> null
// output -> ypred
int predict_final(Response &res)
{
    // DECRYPT CMT!!
    if(res->output == NULL || res->compression == NULL || res->cmt == NULL)
        return ERROR;

    int result = evaluate(res->output, res->compression, res->cmt, res->final_sfk, ACTIVATION, res, 0, -1, ENCRYPT);
    return result;
}

// input_1 -> weights
// output ypred
int predict_train(Response &res)
{
    mpz_t sfk;
    mpz_init(sfk);

    if(res->output == NULL || res->compression == NULL || res->cmt == NULL || res->input == NULL)
        return ERROR;
    row_inner_product(sfk, res->input, sk_1.data());

    // DECRYPT CMT!!
    int result = evaluate(res->output, res->compression, res->cmt, sfk, ACTIVATION, res, 0, -1, NO_ENCRYPT);

    mpz_clear(sfk);
    return result;
}

/**
 * Setup the secret keys required to compute the function secret
 * This method is only for testing
 * In a practical scenario, the client is expected to encrypt these values and send them to the enclave
 */
int setup_secret_key(Response &res)
{
    // Input_1 -> data for secret key
    if(res->input == NULL || res->key_id < 1 || res->key_id > 2)
        return ERROR;
    if(res->key_id == 1)
        sk_1.set_key(res->input);
    else
        sk_2.set_key(res->input);
    print_matrix_e(sk_1.data());
    return COMPLETED;
}

/**
 * Setup the secret function key
 * @params : req (input_1 = weights)
 * @return : store result in req->final_sfk
 */
int set_sfk(Response &res)
{
    if(res->input == NULL)
        return ERROR;
    mpz_t sfk;
    mpz_init(sfk);
    row_inner_product(sfk, res->input, sk_1.data());
    mpz_set(res->final_sfk, sfk);
    return COMPLETED;
}

int test_routine(Response res)
{
    if(res->output == NULL || res->input == NULL)
        return ERROR;

    mpz_t x;
    mpz_init(x);
    mpz_set_si(x, 1);

    printf_enclave("Input Enclave : 0x% " PRIxPTR "\n", (uintptr_t)(void*)res->input);
    printf_enclave("Output Enclave : 0x%" PRIxPTR"\n", (uintptr_t)(void*)res->output);

    printf_enclave("\nInput: \n");
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            mpz_set(mat_element(res->input, i, j), mat_element(res->input, i, j));
        }
    }
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            printf_enclave("%lf ", mpz_get_d(mat_element(res->input, i, j)));
        }
        printf_enclave("\n");
    }

    Matrix tmp = mat_init(res->input->rows, res->input->cols);
    printf_enclave("\nTMP: \n");
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            mpz_set(mat_element(tmp, i, j), mat_element(res->input, i, j));
        }
    }

    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            printf_enclave("%lf ", mpz_get_d(mat_element(tmp, i, j)));
        }
        printf_enclave("\n");
    }

    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            mpz_add(mat_element(tmp, i, j), mat_element(res->input, i, j), x);
        }
    }

    printf_enclave("\nOutput: \n");
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            mpz_set(mat_element(res->output, i, j), mat_element(res->input, i, j));
        }
    }
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            printf_enclave("%lf ", mpz_get_d(mat_element(res->output, i, j)));
        }
        printf_enclave("\n");
    }

    printf_enclave("\nTMP Modified: \n");
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            printf_enclave("%lf ", mpz_get_d(mat_element(tmp, i, j)));
        }
        printf_enclave("\n");
    }

    printf_enclave("\nOutput Modified: \n");
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            mpz_add(mat_element(res->output, i, j), mat_element(tmp, i, j), x);
        }
    }
    for(int i = 0; i < res->input->rows; i++)
    {
        for(int j = 0; j < res->input->cols; j++)
        {
            printf_enclave("%lf ", mpz_get_d(mat_element(res->output, i, j)));
        }
        printf_enclave("\n");
    }
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
        if (req == NULL)
            __asm__("pause");
        else
        {
            //printf("Received request\n");
            //req->status = SCHEDULED;
            Response res = deserialize_request(req);
            switch(req->job_id)
            {
                case GENERATE_LOOKUP_TABLE:
                {
                    req->status = compute_lookup_table(res);
                    printf_enclave("Status : %d\n", req->status);
                    free(res);
                    break;
                }
                case FINAL_PREDICTION:
                {
                    req->status = predict_final(res);
                    break;
                }
                case TRAIN_PREDICTION:
                {
                    req->status = predict_train(res);
                    break;
                }
                case WEIGHT_UPDATE:
                {
                    req->status = update_weights(res);
                    break;
                }
                case SET_FE_SECRET_KEY:
                {
                    req->status = setup_secret_key(res);
                    break;
                }
                case SET_SFK:
                {
                    req->status = set_sfk(res);
                    break;
                }
                case GET_PUB_KEY:
                    break;
                case TEST_DATA_TRANSMISSION:
                {
                    req->status = test_routine(res);
                    break;
                }
            }
        }
    }
    return SUCCESS;
}

void printf_enclave(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
}

void print_matrix_e(Matrix mat)
{
    ocall_print_matrix(serialize_matrix(mat));
}