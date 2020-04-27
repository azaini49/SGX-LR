#pragma once

// #include <gmp.h>
#include "../tools/sgx_tgmp.h"
#include "../tools/matrix_shared.h"
#include "../include/shared.h"
#include "../tools/secret_key.h"
#include "../tools/gmpxx.h"


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

#define ACTIVATION 0
#define NO_ACTIVATION 1

struct MapComp
{
    bool operator() (const mpz_class a, const mpz_class b) const
    {
        if(a < b)
            return true;
        return false;
    }
};

void printf_enclave(const char *fmt, ...);
void print_matrix_e(const Matrix mat);
void print_ematrix_e(const E_Matrix mat);
void print_mpz(mpz_t m);