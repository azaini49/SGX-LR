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
    // Generate random nonce
    mpz_t nonce;
    mpz_init(nonce);

    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);

    int row = tid;
    while(row < plaintext->rows)
    {

        //mpz_urandomm(nonce, state, enc.ctx->p);
        mpz_set_ui(nonce, 5); // jess code

        mpz_add_ui(nonce, nonce, 2);
        mpz_mod(nonce, nonce, enc.ctx->p);

        // Set ct0 = g^r
        mpz_powm(mat_element(commitment, row, 0), enc.ctx->g, nonce, enc.ctx->p);


        for(int col = 0; col < plaintext->cols; col++)
        {
            std::cout << "encrypting thread " << tid << " row :"<< row << " col: "<< col << " val: " << mpz_get_si(mat_element(plaintext, row, col)) <<"\n";
            mpz_powm(tmp, mat_element(enc.pk.data(), 0, col), nonce, enc.ctx->p); // h_i^r in tmp
            std::cout << "h^r (ct0): " << mpz_get_si(tmp) <<"\n";

            mpz_set(mat_element(ciphertext, row, col), tmp); // ct_i = h_i^r


            mpz_powm(tmp, enc.ctx->g, mat_element(plaintext, row, col), enc.ctx->p); // tmp = g^xi
            std::cout << "g^xi : " << mpz_get_si(tmp) <<"\n";

            mpz_mul(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), tmp); // ct_i = h_i^r * g^xi
            std::cout << "h_i^r * g^xi : " << mpz_get_si(mat_element(ciphertext, row, col)) <<"\n";

            mpz_mod(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), enc.ctx->p);
            std::cout << "encrypted " << mpz_get_si(mat_element(ciphertext, row, col)) <<"\n";
        }
        row = row + numThreads;
    }
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
