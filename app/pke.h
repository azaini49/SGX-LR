#pragma once

#include "matrix.h"
#include "../tools/secret_key.h"
#include "pub_key.h"

class PubKeyEncr
{
    public:
        mpz_t N;
        mpz_t g;
        PubKeyEncr(int security_level);
        void generate_key(int security_level);
        void encrypt(mpz_t ciphertext, mpz_t m);
        void decrypt(mpz_t dest, mpz_t ciphertext);

    private:
        PubKeyEncr() = delete;
        int sk_len;
        int security_level;
        mpz_t p;
        mpz_t q;
        mpz_t lambda;
        void generate_sk();
        void generate_pk();
        void generate_key_util(PubKeyEncr &pke, int security_level, int tid, int numThreads);
        void generate_safe_prime(int bits, mpz_t prime);
        void encrypt_util(PubKeyEncr &pke, mpz_t ciphertext, mpz_t plaintext, gmp_randstate_t state, int tid, int numThreads);
        void decrypt_util(PubKeyEncr &pke, mpz_t dest, mpz_t ciphertext, int tid, int numThreads);
};