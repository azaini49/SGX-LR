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
        mpz_t s;
        mpz_t Ns;
        int Mx;
        int My;
        int security_level;
        static std::shared_ptr<Context> Create(int security_level, int Mx, int My);
        Context() = delete;
    private:
        Context(int security_level, int Mx, int My);
        Context(const Context &copy) = delete;
        Context(Context &&source) = delete;
        Context &operator =(const Context &assign) = delete;
        Context &operator =(Context &&assign) = delete;
        void generate_safe_prime(int bits, mpz_t prime);
        void generator();
};
