#include "encryptor.h"
#include <thread>
#include <iostream>

/**
 * Constructor for Encryptor
 * @params : context, FE pk
 */
Encryptor::Encryptor(std::shared_ptr<Context> context, const Public_Key &pub_key)
    :ctx(context), pk(pub_key)
{
    if(ctx == NULL)
        throw std::invalid_argument("Context cannot be null!");
}

// Utility function to encrypt data
void Encryptor::encrypt_util(Encryptor &enc, Matrix ciphertext, Matrix commitment, const Matrix plaintext, gmp_randstate_t state, int tid, int numThreads)
{
    mpz_t norm;
    mpz_init(norm);
    mpz_t tmp2;
    mpz_init(tmp2);
    mpz_set(norm, 0);
    while(row < plaintext->rows)
    {
        for(int col = 0; col < plaintext->cols; col++) {
            mpz_pow_ui(tmp, mat_element(plaintext, row, col), 2);
            mpz_add(norm, tmp);
        }
        mpz_sqrt (norm, norm);      
        if (mpz_cmp(norm, ctx->Mx) > 0) {
            throw std::invalid_argument("Plaintext value too large!");
        }

        row = row + numThreads;
    }
    mpz_clear(norm);
    mpz_clear(tmp2);
    
    // Generate random nonce - let nonce equal the value r in scheme
    mpz_t nonce;
    mpz_init(nonce);

    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);

    mpz_t nd4;
    mpz_init(nd4);
    mpz_fdiv_q_ui(nd4, enc.ctx->N, 4);

    int row = tid;
    while(row < plaintext->rows)
    {

        //mpz_urandomm(nonce, state, nd4); TEMP
        //mpz_add_ui(nonce, nonce, 2);
        mpz_set_si(nonce, 3); // TEMP

        mpz_powm(mat_element(commitment, row, 0), enc.ctx->g, nonce, enc.ctx->Ns); // Set b = g^r

        // compute cti as (1 + N)^xi times pki^r in group G (assuming p is the modulus for group G)
        for(int col = 0; col < plaintext->cols; col++)
        {
            mpz_powm(tmp, mat_element(enc.pk.data(), 0, col), nonce, enc.ctx->Ns); // power hp^r
            mpz_set(mat_element(ciphertext, row, col), tmp);
            mpz_powm(tmp, enc.ctx->N + 1, mat_element(plaintext, row, col), enc.ctx->Ns);// power (N + 1)^xi
            mpz_mul(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), tmp); // mult (N + 1)^xi * hp^r
            mpz_mod(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), enc.ctx->Ns);// mod prev val
        }
        row = row + numThreads;
    }
    mpz_clear(nd4);
    mpz_clear(nonce);
    mpz_clear(tmp);
}

/**
 * Encrypt plaintext to get the corresponding ciphertext and commitment
 * @params : plaintext matrix
 * @return : ciphertext, commitment
 */
void Encryptor::encrypt(Matrix ciphertext, Matrix commitment, const Matrix plaintext)
{
    if(ciphertext->rows != plaintext->rows || ciphertext->cols != plaintext->cols)
        throw std::invalid_argument("Ciphertext and plaintext dimentions do not match!");
    if(commitment->rows != plaintext->rows || commitment->cols != 1)
        throw std::invalid_argument("Invalid dimensions for commitment matrix");

    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    // Define threadpool
    int numThreads = plaintext->rows;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(encrypt_util, std::ref(*this), std::ref(ciphertext), std::ref(commitment), std::ref(plaintext), state, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();


}
