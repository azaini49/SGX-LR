#include "secret_key.h"
#include <thread>
#include <mutex>

Secret_Key::Secret_Key(){}

Secret_Key::Secret_Key(int key_len)
    :key_len(key_len)
{
    this->data_= mat_init(1, key_len);
}

Secret_Key::Secret_Key(const Secret_Key &copy)
{
    this->key_len = copy.key_len;
    this->data_= mat_init(1, key_len);
    mat_copy(this->data_, copy.data_);
}

Secret_Key::Secret_Key(const Matrix data)
{
    // if(data->rows != 1)
    //     throw std::invalid_argument("Invalid Dimensions! Data dimension should be 1 x n");
    this->key_len = data->cols;
    this->data_= mat_init(1, key_len);
    mat_copy(this->data_, data);
}

Secret_Key::~Secret_Key()
{
    //delete_matrix(this->data_);
}

void Secret_Key::init(int key_len)
{
    this->key_len = key_len;
    this->data_= mat_init(1, key_len);
    this->data_cca_j1_= mat_init(1, key_len);
    this->data_cca_j2_= mat_init(1, key_len);
}

void Secret_Key::set_key(const Matrix data, const Matrix data_cca_j1, const Matrix data_cca_j2)
{
    this->key_len = data->cols;
    this->data_= mat_init(1, key_len);
    this->data_cca_j1_= mat_init(1, key_len);
    this->data_cca_j2_= mat_init(1, key_len);
    mat_copy(this->data_, data);
    mat_copy(this->data_cca_j1_, data);
    mat_copy(this->data_cca_j2_, data);
}


int Secret_Key::length()
{
    return this->key_len;
}

void Secret_Key::print()
{
    //print_matrix(this->data_);
}

const Matrix Secret_Key::data() const
{
    return this->data_;
}
