#pragma once
// #include <gmp.h>
// #include <gmpxx.h>
#include "../tools/sgx_tgmp.h"
#include <memory>

class Context
{
    public:
        mpz_t N;
        mpz_t g;
        int security_level;
        static std::shared_ptr<Context> Create(int security_level, mpz_t prime = NULL, mpz_t gen = NULL);
        Context() = delete;
    private:
        Context(int security_level, mpz_t prime = NULL, mpz_t gen = NULL);
        Context(const Context &copy) = delete;
        Context(Context &&source) = delete;
        Context &operator =(const Context &assign) = delete;
        Context &operator =(Context &&assign) = delete;
        void generate_safe_prime(int bits);
        void generator();
};
