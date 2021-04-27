#include "pub_key.h"
#include "matrix.h"
#include <iostream>

Public_Key::Public_Key(){}

Public_Key::Public_Key(const Public_Key &copy)
{
    this->key_len = copy.key_len;
    this->data_= mat_init(1, key_len);
    this->data_cca_j1_ = mat_init(1, key_len);
    this->data_cca_j2_ = mat_init(1, key_len);
    mat_copy(this->data_, copy.data_);
    mat_copy(this->data_cca_j1_, copy.data_cca_j1_);
    mat_copy(this->data_cca_j2_, copy.data_cca_j2_);
}

Public_Key::~Public_Key()
{
    //delete_matrix(this->data_);
}

void Public_Key::init(int key_len)
{
    this->key_len = key_len;
    this->data_ = mat_init(1, key_len);
    this->data_cca_j1_ = mat_init(1, key_len);
    this->data_cca_j2_= mat_init(1, key_len);
}

const Matrix Public_Key::data() const
{
    return this->data_;
}

const Matrix Public_Key::data_cca_j1() const
{
    return this->data_cca_j1_;
}

const Matrix Public_Key::data_cca_j2() const
{
    return this->data_cca_j2_;
}


int Public_Key::length()
{
    return this->key_len;
}

void Public_Key::print()
{
    print_matrix(this->data_);
}
