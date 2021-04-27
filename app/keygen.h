#pragma once

#include "context.h"
#include "../tools/secret_key.h"
#include "pub_key.h"

class Keygen
{
    public:
        std::shared_ptr<Context> context;
        bool cca = false;
        Keygen(std::shared_ptr<Context> context, int sk_len, bool cca);
        Keygen(std::shared_ptr<Context> context, const Secret_Key &secret_key_copy, bool cca);
        const Public_Key& public_key() const;
        const Secret_Key& secret_key() const;
        void key_der(const Matrix y, mpz_t hky, mpz_t hky_cca_j1=NULL, mpz_t sky_cca_j2=NULL);

    private:
        Public_Key pk;
        Secret_Key sk;
        int msk_len;
        bool gen_secret_key = true;
        Keygen() = delete;
        void generate_sk();
        void generate_pk();
        static void generate_sk_util(Keygen &gen, mpz_t limit, mpz_t limit_cca, int tid, gmp_randstate_t state, int numThreads);
        static void generate_pk_util(Keygen &gen, int tid, int numThreads);
        static void key_der_util(Keygen &kg, mpz_t hky, mpz_t hky_cca_j1, mpz_t hky_cca_j2, const Matrix y, int tid, int numThreads);

};
