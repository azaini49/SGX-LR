#pragma once

#include "matrix.h"
#include "context.h"

class Secret_Key
{
    private:
        Matrix data_;
        int key_len;
    public:
        Secret_Key();
        Secret_Key(int key_len);
        Secret_Key(const Secret_Key &copy);
        Secret_Key(const Matrix &data);
        ~Secret_Key();
        void init(int key_len);
        int length();
        void print();
        const Matrix &data() const;
        friend class Keygen;
};