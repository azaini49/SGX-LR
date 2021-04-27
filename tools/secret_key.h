#pragma once

#include "matrix_shared.h"

class Secret_Key
{
    private:
        Matrix data_;
        Matrix data_cca_j1_;
        Matrix data_cca_j2_;
        int key_len;
    public:
        Secret_Key();
        Secret_Key(int key_len);
        Secret_Key(const Secret_Key &copy);
        Secret_Key(const Matrix data);
        ~Secret_Key();
        void init(int key_len);
        void set_key(const Matrix data, const Matrix data_cca_j1=NULL, const Matrix data_cca_j2=NULL);
        int length();
        void print();
        const Matrix data() const;
        friend class Keygen;

};
