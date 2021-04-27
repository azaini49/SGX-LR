#pragma once

#include "pub_key.h"
#include "matrix.h"

class Encryptor
{
    public:
        Encryptor(std::shared_ptr<Context> context, const Public_Key &pub_key);
        void encrypt(Matrix ciphertext, Matrix commitment, const Matrix plaintext, const Matrix cca_ct=NULL);
    private:
        std::shared_ptr<Context> ctx;
        Public_Key pk;
        Encryptor() = delete;
        static void encrypt_util(Encryptor &enc, Matrix ciphertext, Matrix commitment, Matrix cca_ct, const Matrix plaintext, gmp_randstate_t state, int tid, int numThreads);
};
