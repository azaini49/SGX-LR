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

// const sgx_ec256_public_t app_pk =
// {
//     0x82, 0xcb, 0x6f, 0x41, 0x3a, 0xd4, 0xfa, 0x57,
//     0x6c, 0xc4, 0x1b, 0x77, 0xf6, 0xd9, 0x51, 0xc1,
//     0xbc, 0x17, 0x7a, 0x88, 0xd0, 0x2e, 0x94, 0xd6,
//     0x91, 0xa3, 0x1d, 0x75, 0xc, 0xbf, 0xa9, 0xca,
//     0x8, 0x6c, 0xf3, 0x78, 0x92, 0xdb, 0x2f, 0x52,
//     0x0, 0x44, 0x20, 0xd6, 0xa, 0xd3, 0x58, 0x3,
//     0xb2, 0x35, 0xda, 0xe2, 0x1b, 0xdb, 0x2b, 0xd2,
//     0xb0, 0xaf, 0x5e, 0x29, 0xc8, 0xb4, 0x93, 0x41
// };

struct MapComp
{
    bool operator() (const mpz_class a, const mpz_class b) const
    {
        if(a < b)
            return true;
        return false;
    }
};

mpz_t sfk;
void printf_enclave(const char *fmt, ...);
void print_matrix_e(const Matrix mat);
void print_ematrix_e(const E_Matrix mat);
void print_mpz(mpz_t m);