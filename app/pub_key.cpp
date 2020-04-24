#include "pub_key.h"
#include "matrix.h"

Public_Key::Public_Key(){}

Public_Key::Public_Key(const Public_Key &copy)
{
    this->key_len = copy.key_len;
    this->data_= mat_init(1, key_len);
    mat_copy(this->data_, copy.data_);
}

Public_Key::~Public_Key()
{
    delete_matrix(this->data_);
}

void Public_Key::init(int key_len)
{
    this->key_len = key_len;
    this->data_ = mat_init(1, key_len);
}

const Matrix& Public_Key::data() const
{
    return this->data_;
}

int Public_Key::length()
{
    return this->key_len;
}

void Public_Key::print()
{
    print_matrix(this->data_);
}