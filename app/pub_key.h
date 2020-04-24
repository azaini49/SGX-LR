#pragma once

#include "../tools/matrix.h"
#include "context.h"

class Public_Key
{
    private:
        Matrix data_;
        int key_len;
        Public_Key(Matrix mat) = delete;
        Public_Key(int key_len) = delete;
        const Matrix& data() const;
    public:
        Public_Key();
        Public_Key(const Public_Key &copy);
        ~Public_Key();
        int length();
        void init(int key_len);
        void print();
        friend class Keygen;
        friend class Encryptor;
};