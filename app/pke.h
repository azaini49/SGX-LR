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
        static std::shared_ptr<PubKeyEncr> Create(int security_level);
        static void encrypt(std::shared_ptr<PubKeyEncr> pke, mpz_t ciphertext, mpz_t m);
        static void decrypt(std::shared_ptr<PubKeyEncr> pke,mpz_t dest, mpz_t ciphertext);

    private:
        PubKeyEncr() = delete;
        int sk_len;
        int security_level;
        mpz_t p;
        mpz_t q;
        mpz_t lambda;
        mpz_t mu;
        void generate_sk();
        void generate_pk();
        static void generate_safe_prime(int bits, mpz_t prime);
        static void encrypt_util(std::shared_ptr<PubKeyEncr> pke, mpz_t ciphertext, mpz_t plaintext, gmp_randstate_t state, int tid, int numThreads);
        static void decrypt_util(std::shared_ptr<PubKeyEncr> pke, mpz_t dest, mpz_t ciphertext, int tid, int numThreads);
};
