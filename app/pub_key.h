#pragma once

#include "matrix.h"
#include "context.h"

class Public_Key
{
    private:
        Matrix data_;
        Matrix data_cca_j1_;
        Matrix data_cca_j2_;
        int key_len;
        bool cca = false;
        Public_Key(Matrix mat) = delete;
        Public_Key(int key_len) = delete;
    public:
        Public_Key();
        Public_Key(const Public_Key &copy);
        ~Public_Key();
        int length();
        void init(int key_len);
        void print();
        const Matrix data() const;
        const Matrix data_cca_j1() const;
        const Matrix data_cca_j2() const;
        friend class Keygen;
        friend class Encryptor;
};
