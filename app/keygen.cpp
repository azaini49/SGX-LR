#include "keygen.h"
#include <thread>
#include <stdexcept>

/**
 * Constructer for KeyGen
 * @params : context, lenght of secret key
 */
Keygen::Keygen(std::shared_ptr<Context> context, int sk_len, bool cca)
    :context(context), msk_len(sk_len), cca(cca)
{
    if(context == NULL)
        throw std::invalid_argument("Context is invalid!");

    std::cout << "\n====== GENERATING KEYS =====\n";
    this->sk.init(sk_len);
    this->pk.init(sk_len);
    generate_sk();
    std::cout << "=> Secret Key Made\n";
    generate_pk();
    std::cout << "=> Public Key Made\n";
    this->pk.cca = cca;
}

/**
 * Constructer for KeyGen
 * @params : context, copy of secret key
 */
Keygen::Keygen(std::shared_ptr<Context> context, const Secret_Key &secret_key_copy, bool cca)
    :context(context), sk(secret_key_copy), gen_secret_key(false), cca(cca)
{
    if(context == NULL)
        throw std::invalid_argument("Context is invalid!");
    if(this->msk_len != sk.key_len)
        throw std::invalid_argument("Secret key length does not match the length specified by the context!");
    this->pk.init(this->msk_len);
    generate_pk();
}


// Generate secret key (USE STRONGER RANDOM NUMBER GENERATOR!!!)
void Keygen::generate_sk_util(Keygen &gen, mpz_t limit, mpz_t limit_cca, int tid, gmp_randstate_t state, int numThreads)
{
    int col = tid;
    mpz_t val;
    mpz_init(val);

    while(col < gen.msk_len)
    {
        // Set position in secret key
        mpz_urandomm(val, state, limit);
        mpz_add_ui(val, val, 2);
        //mpz_mod(val, val, gen.context->);

        set_matrix_element(gen.sk.data_, 0, col, val);

        //mpz_powm(val, gen.context->g, val, gen.context->Ns);
        //set_matrix_element(gen.pk.data_, 0, col, val);

        if (gen.cca){
          mpz_urandomm(val, state, limit_cca);
          mpz_add_ui(val, val, 2);
          //mpz_mod(val, val, gen.context->p);

          set_matrix_element(gen.sk.data_cca_j1_, 0, col, val);

          mpz_urandomm(val, state, limit_cca);
          mpz_add_ui(val, val, 2);
          //mpz_mod(val, val, gen.context->p);

          set_matrix_element(gen.sk.data_cca_j2_, 0, col, val);
        }

        col = col + numThreads;
    }
    mpz_clear(val);
}

// Generate secret key
void Keygen::generate_sk()
{

    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    // M
    int M = this->msk_len * 2^this->context->security_level * this->context->Mx * (4 * this->context->Mx)^this->msk_len;
    mpz_t limit;
    mpz_init_set_si(limit, M);

    mpz_mul(limit, limit, this->context->Ns);
    mpz_fdiv_q_ui(limit, limit, 4);
    mpz_sub_ui(limit, limit, 2);

    mpz_t limit_cca;
    mpz_init_set_si(limit_cca, M);

    mpz_mul(limit_cca, limit_cca, this->context->Ns);
    mpz_fdiv_q_ui(limit_cca, limit_cca, 2);
    mpz_sub_ui(limit_cca, limit_cca, 2);

    // Define threadpool
    int numThreads = std::thread::hardware_concurrency();
    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(generate_sk_util, std::ref(*this), limit, limit_cca, i, state, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
    this->gen_secret_key = false;

    mpz_clear(limit);
}

// Generate public key (USE STRONGER RANDOM NUMBER GENERATOR!!!)
void Keygen::generate_pk_util(Keygen &gen, int tid, int numThreads)
{
    int col = tid;
    while(col < gen.msk_len)
    {
        mpz_powm(mat_element(gen.pk.data_, 0, col), gen.context->g, mat_element(gen.sk.data_, 0, col), gen.context->Ns);

        if (gen.cca){
          mpz_powm(mat_element(gen.pk.data_cca_j1_, 0, col), gen.context->g, mat_element(gen.sk.data_cca_j1_, 0, col), gen.context->Ns);
          mpz_powm(mat_element(gen.pk.data_cca_j2_, 0, col), gen.context->g, mat_element(gen.sk.data_cca_j2_, 0, col), gen.context->Ns);
        }

        col = col + numThreads;
    }
}

// Generate public key
void Keygen::generate_pk()
{
    if(gen_secret_key)
        throw std::logic_error("Pub key cannot be generated befor the secret key!");
    if(this->cca && (this->pk.data_cca_j1_ == NULL || this->pk.data_cca_j2_ == NULL)){
        throw std::invalid_argument("Need CCA pk commitment!");
    }

    // Define threadpool
    int numThreads = std::thread::hardware_concurrency();
    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(generate_pk_util, std::ref(*this), i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
}

void Keygen::key_der_util(Keygen &kg, mpz_t hky, mpz_t hky_cca_j1, mpz_t hky_cca_j2, const Matrix y, int tid, int numThreads)
{

    mpz_t norm;
    mpz_init(norm);

    mpz_t tmp2;
    mpz_init(tmp2);

    mpz_set_si(norm, 0);

    int col = tid;

    mpz_t tmp;
    mpz_init_set_si(tmp,0);

    mpz_t tmp_cca;
    mpz_init(tmp_cca);

    while(col < kg.msk_len)
    {
      // SIZE CHECK
      for(int col = 0; col < y->cols; col++) {
          mpz_mul(tmp2, mat_element(y, 0, col), mat_element(y, 0, col));
          mpz_add(norm, norm, tmp2);
      }
      mpz_sqrt(norm, norm);
      if (mpz_cmp_si(norm, kg.context->My) > 0) {
          throw std::invalid_argument("y value too large!");
      }


      // KEY DER
      mpz_mul(tmp, mat_element(y, 0, col), mat_element(kg.sk.data_, 0, col));
      mpz_add(hky, hky, tmp);

      if (kg.cca){
        mpz_set_si(tmp_cca,0);
        mpz_mul(tmp_cca, mat_element(y, 0, col), mat_element(kg.sk.data_cca_j1_, 0, col));
        mpz_add(hky_cca_j1, hky_cca_j1, tmp_cca);

        mpz_set_si(tmp_cca,0);
        mpz_mul(tmp_cca, mat_element(y, 0, col), mat_element(kg.sk.data_cca_j2_, 0, col));
        mpz_add(hky_cca_j2, hky_cca_j2, tmp_cca);
      }

      col = col + numThreads;
    }

    mpz_clear(tmp);
    mpz_clear(tmp_cca);
}

void Keygen::key_der(const Matrix y, mpz_t hky, mpz_t hky_cca_j1, mpz_t hky_cca_j2){
  if(this->cca && (this->pk.data_cca_j1_ == NULL || this->pk.data_cca_j2_ == NULL)){
      throw std::invalid_argument("Need CCA pk commitment!");
  }
  if(this->cca && (this->sk.data_cca_j1_ == NULL || this->sk.data_cca_j2_ == NULL)){
      throw std::invalid_argument("Need CCA sk commitment!");
  }


  // Define threadpool
  int numThreads = std::thread::hardware_concurrency();
  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
      threads[i] = std::thread(key_der_util, std::ref(*this), std::ref(hky), std::ref(hky_cca_j1), std::ref(hky_cca_j2), std::ref(y), i, numThreads);
  }
  for(int i = 0; i < numThreads; i++)
      threads[i].join();

}



// Return the secret key
const Secret_Key& Keygen::secret_key() const
{
    if(gen_secret_key)
        throw std::logic_error("Secret key has not been generated!");
    return this->sk;
}

// Return the public key
const Public_Key& Keygen::public_key() const
{
    if(gen_secret_key)
        throw std::logic_error("Public key has not been generated!");
    return this->pk;
}
