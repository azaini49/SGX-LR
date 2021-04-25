#include "secret_key.h"
#include <thread>

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
    delete_matrix(this->data_);
}

void Secret_Key::init(int key_len)
{
    this->key_len = key_len;
    this->data_= mat_init(1, key_len);
}

void Secret_Key::set_key(const Matrix data)
{
    this->key_len = data->cols;
    this->data_= mat_init(1, key_len);
    mat_copy(this->data_, data);
}


void Secret_Key::key_der_util(mpz_t hky, const Matrix y, int tid, int numThreads)
{

    int col = tid;

    mpz_t tmp;
    mpz_init(tmp);

    mpz_set_si(hky,0);

    while(col < this->key_len)
    {
      mpz_mul(tmp, mat_element(y, 0, col), mat_element(this->data_, 0, col));
      mpz_add(hky, hky, tmp);

      col = col + numThreads;
    }
    mpz_clear(val);

}


void Secret_Key::key_der(mpz_t hky, const Matrix y){


  // Define threadpool
  int numThreads = std::thread::hardware_concurrency();
  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
      threads[i] = std::thread(key_der_util, std::ref(hky), std::ref(y), i, numThreads);
  }
  for(int i = 0; i < numThreads; i++)
      threads[i].join();

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
