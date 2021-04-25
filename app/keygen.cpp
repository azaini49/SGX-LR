#include "keygen.h"
#include <thread>
#include <stdexcept>

/**
 * Constructer for KeyGen
 * @params : context, lenght of secret key
 */
Keygen::Keygen(std::shared_ptr<Context> context, int sk_len)
    :context(context), msk_len(sk_len)
{
    if(context == NULL)
        throw std::invalid_argument("Context is invalid!");
    this->sk.init(sk_len);
    this->pk.init(sk_len);
    generate_sk();
    generate_pk();
}

/**
 * Constructer for KeyGen
 * @params : context, copy of secret key
 */
Keygen::Keygen(std::shared_ptr<Context> context, const Secret_Key &secret_key_copy)
    :context(context), sk(secret_key_copy), gen_secret_key(false)
{
    if(context == NULL)
        throw std::invalid_argument("Context is invalid!");
    if(this->msk_len != sk.key_len)
        throw std::invalid_argument("Secret key length does not match the length specified by the context!");
    this->pk.init(this->msk_len);
    generate_pk();
}


// Generate secret key (USE STRONGER RANDOM NUMBER GENERATOR!!!)
void Keygen::generate_sk_util(Keygen &gen, mpz_t limit, int tid, gmp_randstate_t state, int numThreads)
{
    int col = tid;
    mpz_t val;
    mpz_init(val);

    while(col < gen.msk_len)
    {
        // Set position in secret key
        mpz_urandomm(val, state, limit); // -2 ?

        //mpz_add_ui(val, val, 2);
        //mpz_mod(val, val, gen.context->p);
        set_matrix_element(gen.sk.data_, 0, col, val);

        mpz_powm(val, gen.context->g, val, gen.context->Ns);
        set_matrix_element(gen.pk.data_, 0, col, val);
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

    mpz_t Nso;
    mpz_init(Nso);
    mpz_set(Nso, this->context->Ns);
    mpz_mul(Nso, Nso, this->context->N);

    mpz_mul(limit, limit, Nso);
    mpz_fdiv_q_ui(limit, limit, 4);

    // Define threadpool
    int numThreads = std::thread::hardware_concurrency();
    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(generate_sk_util, std::ref(*this), limit, i, state, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
    this->gen_secret_key = false;

    mpz_clear(Nso);
    mpz_clear(limit);
}

// Generate public key (USE STRONGER RANDOM NUMBER GENERATOR!!!)
void Keygen::generate_pk_util(Keygen &gen, int tid, int numThreads)
{
    int col = tid;
    while(col < gen.msk_len)
    {
        mpz_powm(mat_element(gen.pk.data_, 0, col), gen.context->g, mat_element(gen.sk.data_, 0, col), gen.context->Ns);
        col = col + numThreads;
    }
}

// Generate public key
void Keygen::generate_pk()
{
    if(gen_secret_key)
        throw std::logic_error("Pub key cannot be generated befor the secret key!");

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
