#include "Enclave.h"
#include "enclave_t.h"
#include <math.h>
#include <vector>
#include "../include/Queue.h"
#include <inttypes.h>
#include <map>

#ifndef SUCCESS
#define SUCCESS 1
#endif

#ifndef FAILURE
#define FAILURE -1
#endif

extern int SECURITY_BITS;

// Store the secret keys in the enclave
Secret_Key sk_1;
Secret_Key sk_2;

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

int get_discrete_log(mpz_t &x, mpz_t p)
{
    mpz_class key{x};
    auto it = lookup.begin();
    while(it != lookup.end())
    {
        if(it->first == key)
            mpz_set(x, it->second.get_mpz_t());
        it++;
    }
}

/** Utility function to generate lookup table
 * Currently supports only num_threads = 1
 */
/*int compute_lookup_table(Response res)
{
    mpz_t tmp;
    mpz_init(tmp);

    mpz_class i = 0;
    mpz_class limit = pow(2, res->limit);
    while(i < limit)
    {
        mpz_powm(tmp, res->g, i.get_mpz_t(), res->N);
        lookup[mpz_class{tmp}] = i;
        mpz_invert(tmp, tmp, res->N);
        lookup[mpz_class{tmp}] = -i;
        i = i + 1;
    }
    mpz_clear(tmp);
    return COMPLETED;
}*/

/**
 * Decrypt commitments and compute the FE result
 * Currently commitments are not encfypted
 * Supports num_threads = 1
 * input -> pk
 */
int evaluate(const mpz_t sfk, int activation, Response res, int start, int end, int mode)
{
    if(res->output == NULL || res->compression == NULL || res->cmt == NULL)
        return ERROR;
    int row = 0;
    mpz_t ct0;
    mpz_init(ct0);

    mpz_t tmp;
    mpz_init(tmp);

    if(end == -1)
        end = res->output->rows - 1;

    // Generate random nonce
    mpz_t nonce;
    mpz_init(nonce);

    mpz_t tmp2;
    mpz_init2(tmp2, SECURITY_BITS);

    mpz_t tmp3;
    mpz_init2(tmp3, SECURITY_BITS);

    mpz_t tmp4;
    mpz_init2(tmp4, SECURITY_BITS);

    mpz_t nd4;
    mpz_init(nd4);
    mpz_fdiv_q_ui(nd4, res->N, 4);
    mpz_sub_ui(nd4,nd4,2); 

    mpz_t Nsq;
    mpz_init(Nsq);
    mpz_mul(Nsq, res->N, res->N);

    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    while(row < end + 1)
    {
        mpz_set(ct0, mat_element(res->cmt, row, 0));
        mpz_powm(ct0, ct0, sfk, res->Ns);
        mpz_invert(ct0, ct0, res->Ns);
        mpz_set_si(tmp, 1);

        mpz_mul(tmp, mat_element(res->compression, row, 0), ct0);
        mpz_mod(tmp, tmp, res->Ns);
        //get_discrete_log(tmp, res->N);
        mpz_sub_ui(tmp, tmp, 1); // subtract 1 from compression / ct0^sfk
        mpz_tdiv_q(tmp, tmp, res->N); // divide compression / ct0^sfk -1  by N
        mpz_mod(tmp, tmp, res->Ns); // take mod of prev value



        if(activation == ACTIVATION)
            sigmoid(tmp, mpz_get_d(tmp));

        if(mode == ENCRYPT)
        {
            mpz_urandomm(nonce, state, nd4);
            mpz_add_ui(nonce, nonce, 2);
            //mpz_mod(nonce, nonce, res->N);

            // Set ct0 = g^r
         
	    mpz_powm(tmp4, res->g, nonce, res->Ns);
            mpz_set(mat_element(res->cmt, row, 0), tmp4);	    
	    //mpz_powm(mat_element(res->cmt, row, 0),res->g, nonce, res->N);

            mpz_powm(tmp2, mat_element(res->input, 0, 0), nonce, res->Ns); // power hp^r
            mpz_set(tmp3, tmp2);

            mpz_mul(tmp2, res->N, tmp);// power (N + 1)^xi
            mpz_add_ui(tmp2, tmp2, 1);

            mpz_mul(tmp3, tmp3, tmp2); // mult (N + 1)^xi * hp^r
            mpz_mod(tmp3, tmp3, res->Ns);// mod prev val
	    mpz_set(mat_element(res->output, row, 0), tmp3);

        }
        else
            mpz_set(mat_element(res->output, row, 0), tmp);
        row = row + 1;
    }
    /*
    mpz_clear(tmp);
    mpz_clear(tmp2);
    mpz_clear(tmp3);
    mpz_clear(nonce);
    mpz_clear(ct0);
    */
    return COMPLETED;
}



/**
 * Decrypt commitments and compute the FE result
 * Currently commitments are not encfypted
 * Supports num_threads = 1
 */
int evaluate_e(E_Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t sfk, int activation, Response res, int start, int end, int mode)
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
        mpz_powm(ct0, ct0, sfk, res->Ns);
        mpz_invert(ct0, ct0, res->Ns);
        mpz_set_si(tmp, 1);

        mpz_mul(tmp, mat_element(compression, row, 0), ct0);
        mpz_mod(tmp, tmp, res->Ns);

        //get_discrete_log(tmp, res->N);
        mpz_sub_ui(tmp, tmp, 1); // subtract 1 from compression / ct0^sfk
        mpz_tdiv_q(tmp, tmp, res->N); // divide compression / ct0^sfk -1  by N
        mpz_mod(tmp, tmp, res->Ns); // take mod of prev value

        if(activation == ACTIVATION)
            sigmoid(tmp, mpz_get_d(tmp));
        mpz_set(mat_element(dest, row, 0), tmp);
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
int update_weights(Response res)
{
    // Initialize sfk_update
    mpz_t sfk_update;
    mpz_init(sfk_update);
    struct e_matrix update;
    struct e_matrix update_trans;
    setup_matrix(&update, res->output->cols, res->output->rows);

    // Compute update to be made
    int stat = row_inner_product(sfk_update, sk_2.data(), res->input, -1, 0, 0, res->start_idx, res->start_idx + res->batch_size - 1);
    if(stat == ERROR)
    {
        printf_enclave("error\n");
        mpz_clear(sfk_update);
        return ERROR;
    }

    evaluate_e(&update, res->compression, res->cmt, sfk_update, NO_ACTIVATION, res, 0, -1, NO_ENCRYPT);

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
            mpf_class tmp = ((mpz_get_si(wt)*res->alpha) + mpz_get_si(mat_element2(update, 0 ,col)))*res->learning_rate;
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
    return COMPLETED;
}

// input_1 -> pk
// output -> ypred
int predict_final(Response res)
{
    if(res->input == NULL || res->output == NULL || res->compression == NULL || res->cmt == NULL)
        return ERROR;
    int result = evaluate(sfk, ACTIVATION, res, 0, -1, NO_ENCRYPT); // changed Jess
    return result;
}

// input_1 -> weights
// output ypred
int predict_train(Response res)
{
    if(res->output == NULL || res->compression == NULL || res->cmt == NULL || res->input == NULL)
        return ERROR;
    row_inner_product(sfk, res->input, sk_1.data());
    int result = evaluate(sfk, ACTIVATION, res, 0, -1, NO_ENCRYPT);

    return result;
}

/**
 * Setup the secret keys required to compute the function secret
 * This method is only for testing
 * In a practical scenario, the client is expected to encrypt these values and send them to the enclave
 */
int setup_secret_key(Response res)
{
    // Input_1 -> data for secret key
    if(res->input == NULL || res->key_id < 1 || res->key_id > 2)
        return ERROR;
    if(res->key_id == 1)
    {
        sk_1.set_key(res->input);
    }
    else
    {
        sk_2.set_key(res->input);
    }
    return COMPLETED;
}


/**
 * Setup the secret function key
 * @params : req (input_1 = weights)
 * @return : store result in req->final_sfk
 */
int set_sfk(Response res, Request req)
{
    if(res->input == NULL)
        return ERROR;
    row_inner_product(sfk, res->input, sk_1.data());
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

    // Initialize final sfk
    mpz_init(sfk);

    // Define task queue
    Queue* task_queue = (Queue*)arg;
    bool shutdown = false;

    // Start listening for request
    while(!shutdown)
    {
        Request req = task_queue->dequeue();
        if (req == NULL)
            __asm__("pause");
        else
        {
            struct response res;
	        init_response(&res);
            deserialize_request(&res, req);
            switch(req->job_id)
            {
                case GENERATE_LOOKUP_TABLE:
                {
                    //req->status = compute_lookup_table(&res);
                    break;
                }
                case FINAL_PREDICTION:
                {
                    req->status = predict_final(&res);
                    break;
                }
                case TRAIN_PREDICTION:
                {
                    req->status = predict_train(&res);
                    break;
                }
                case WEIGHT_UPDATE:
                {
                    req->status = update_weights(&res);
                    break;
                }
                case SET_FE_SECRET_KEY:
                {
                    req->status = setup_secret_key(&res);
                    break;
                }
                case SET_SFK:
                {
                    req->status = set_sfk(&res, req);
                    break;
                }
                case EXIT_ENCLAVE:
                {
                    shutdown = true;
                    req->status = COMPLETED;
                    break;
                }
                case GET_PUB_KEY:
                    break;
            }
        }
    }
    return SUCCESS;
}

void print_mpz(mpz_t m)
{
    char* str = mpz_get_str(NULL, 10, m);
    printf_enclave("%s\n", str);
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

void print_matrix_e(const Matrix mat)
{
   for(int i = 0; i < mat->rows; i++)
   {
        for(int j = 0; j < mat->cols; j++)
        {
            char* str = mpz_get_str(NULL, 10, mat_element(mat, i, j));
            printf_enclave("%s ", str);
        }
        printf_enclave("\n");
   }
   printf_enclave("\n");
}

void print_ematrix_e(const E_Matrix mat)
{
   for(int i = 0; i < mat->rows; i++)
   {
        for(int j = 0; j < mat->cols; j++)
        {
            char* str = mpz_get_str(NULL, 10, mat_element(mat, i, j));
            printf_enclave("%s ", str);
        }
        printf_enclave("\n");
   }
   printf_enclave("\n");
}
