#pragma once

#include "matrix.h"
#include "../tools/secret_key.h"
#include "pub_key.h"

class PubKeyEncr
{
    public:
        mpz_t N;
        mpz_t g;
        PubKeyEncr(int sk_len, int security_level);
        void generate_key(int security_level);
        void encrpt(mpz_t ciphertext, mpz_t m, int start = 0, int end = -1);
        void decrypt(Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t &sfk, int start = 0, int end = -1);

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
        void encrypt_util(PubKeyEncr &pke, Matrix compression, const Matrix ciphertext, const Matrix inp, int start, int end, int tid, int numThreads);
        void decrypt_util(PubKeyEncr &pke, Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t &sfk, int start, int end, int tid, int numThreads);
};