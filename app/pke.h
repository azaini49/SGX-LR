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
        static void encrypt(std::shared_ptr<PubKeyEncr> pke, Matrix ciphertext, Matrix plaintext);
        static void decrypt(std::shared_ptr<PubKeyEncr> pke, Matrix dest, Matrix ciphertext);

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
        static void generate_guess_util(int bits, mpz_t prime);
        static void check_prime_util(mpz_t isPrime, mpz_t prime);
        static void generate_safe_prime(mpz_t p, mpz_t q, int bits);
        static void encrypt_util(std::shared_ptr<PubKeyEncr> pke, Matrix ciphertext, Matrix plaintext, gmp_randstate_t state, int tid, int numThreads);
        static void decrypt_util(std::shared_ptr<PubKeyEncr> pke, Matrix dest, Matrix ciphertext, int tid, int numThreads);
};
