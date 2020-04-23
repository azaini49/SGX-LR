#pragma once

#include <gmp.h>
#include <gmpxx.h>
#include "../app/matrix.h"
#include "../app/logistic_regression.h"
#include "../app/evaluator.h"
#include "../app/context.h"
#include <sgx_tgmp.h>


/***************************************************
 * Enclave return codes
 ***************************************************/
#define RET_SUCCESS 0
#define ERR_PASSWORD_OUT_OF_RANGE 1
#define ERR_WALLET_ALREADY_EXISTS 2
#define ERR_CANNOT_SAVE_WALLET 3
#define ERR_CANNOT_LOAD_WALLET 4
#define ERR_WRONG_MASTER_PASSWORD 5
#define ERR_WALLET_FULL 6
#define ERR_ITEM_DOES_NOT_EXIST 7
#define ERR_ITEM_TOO_LONG 8
#define ERR_FAIL_SEAL 9
#define ERR_FAIL_UNSEAL 10

// Store the secret keys in the enclave
std::unique_ptr<Secret_Key> sk_1;
std::unique_ptr<Secret_Key> sk_2;


struct MapComp
{
    bool operator() (const mpz_class a, const mpz_class b) const
    {
        int flag = mpz_cmp(a.get_mpz_t(), b.get_mpz_t());
        if(flag < 0)
            return true;
        return false;
    }
};

// void ecall_get_discrete_log(mpz_t x, std::shared_ptr<Context> ctx);

// void ecall_lookup_table_util(std::shared_ptr<Context> ctx, mpz_class limit, int tid, int numThreads);

// void ecall_compute_lookup_table(std::shared_ptr<Context> ctx, int bound = 15);

// void ecall_update_weights_util(Logistic_Regression &log_reg, Matrix &update, float alpha, float lr, mpz_class mod, int tid, int numThreads);

// void ecall_update_weights(Logistic_Regression &mdl, Matrix &update_compress, Matrix &cmt, Evaluator &eval, Matrix &training_error,
//                 float alpha, float learning_rate, int start_idx, int batch_size, int update_r, int update_c);

// void ecall_enclave_prediction(Logistic_Regression &mdl, Matrix &ypred, Matrix &compression, Matrix &cmt, Evaluator &eval);

// void ecall_set_secret_key(std::shared_ptr<Secret_Key> sk, int id);
