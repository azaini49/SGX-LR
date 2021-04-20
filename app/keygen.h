#pragma once

#include "context.h"
#include "../tools/secret_key.h"
#include "pub_key.h"

class Keygen
{
    public:
        std::shared_ptr<Context> context;
        Keygen(std::shared_ptr<Context> context, int sk_len);
        Keygen(std::shared_ptr<Context> context, const Secret_Key &secret_key_copy);
        const Public_Key& public_key() const;
        const Secret_Key& secret_key() const;
    
    private:
        Public_Key pk;
        Secret_Key sk;
        int msk_len;
        bool gen_secret_key = true;
        Keygen() = delete;
        void generate_sk();
        void generate_pk();
        static void generate_sk_util(Keygen &gen, int tid, gmp_randstate_t state, int numThreads);
        static void generate_pk_util(Keygen &gen, int tid, int numThreads);
};
