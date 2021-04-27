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
void Encryptor::encrypt_util(Encryptor &enc, Matrix ciphertext, Matrix commitment, Matrix cca_ct, const Matrix plaintext, gmp_randstate_t state, int tid, int numThreads)
{
    int row = tid;

    mpz_t norm;
    mpz_init(norm);

    mpz_t tmp2;
    mpz_init(tmp2);

    mpz_set_si(norm, 0);

    // Generate random nonce - let nonce equal the value r in scheme
    mpz_t nonce;
    mpz_init(nonce);

    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);

    mpz_t nd4;
    mpz_init(nd4);
    mpz_fdiv_q_ui(nd4, enc.ctx->N, 4);
    mpz_sub_ui(nd4,nd4,2);


    while(row < plaintext->rows)
    {
        // SIZE CHECK
        for(int col = 0; col < plaintext->cols; col++) {
            mpz_mul(tmp2, mat_element(plaintext, row, col), mat_element(plaintext, row, col));
            mpz_add(norm, norm, tmp2);
        }
        mpz_sqrt(norm, norm);
        if (mpz_cmp_si(norm, enc.ctx->Mx) > 0) {
            throw std::invalid_argument("Plaintext value too large!");
        }

        // ENCRYPT
        mpz_urandomm(nonce, state, nd4);
        mpz_add_ui(nonce, nonce, 2);

        mpz_powm(mat_element(commitment, row, 0), enc.ctx->g, nonce, enc.ctx->Ns); // Set b = g^r

        // compute cti as (1 + N)^xi times pki^r in group G (assuming p is the modulus for group G)
        for(int col = 0; col < plaintext->cols; col++)
        {
            mpz_powm(tmp, mat_element(enc.pk.data(), 0, col), nonce, enc.ctx->Ns); // power hp^r
            mpz_set(mat_element(ciphertext, row, col), tmp);
            mpz_mul(tmp, enc.ctx->N, mat_element(plaintext, row, col));// power (N + 1)^xi
            mpz_add_ui(tmp, tmp, 1);
            mpz_mul(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), tmp); // mult (N + 1)^xi * hp^r
            mpz_mod(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), enc.ctx->Ns);// mod prev val

            if (enc.pk.cca){ // TODO check
                // hp_j1 * r
                mpz_mul(tmp, mat_element(enc.pk.data_cca_j1(), 0, col), nonce);

                // hp_j2 * r
                mpz_mul(tmp2, mat_element(enc.pk.data_cca_j2(), 0, col), nonce);

                // sum
                mpz_add(tmp, tmp, tmp2);

                // g ^ sum
                mpz_powm(mat_element(cca_ct, row, col), enc.ctx->g, tmp, enc.ctx->Ns); // Set b = g^r

            }


        }

        row = row + numThreads;
    }
    mpz_clear(nd4);
    mpz_clear(nonce);
    mpz_clear(tmp);
    mpz_clear(norm);
    mpz_clear(tmp2);

}

/**
 * Encrypt plaintext to get the corresponding ciphertext and commitment
 * @params : plaintext matrix
 * @return : ciphertext, commitment
 */
void Encryptor::encrypt(Matrix ciphertext, Matrix commitment, const Matrix plaintext, const Matrix cca_ct)
{
    if(ciphertext->rows != plaintext->rows || ciphertext->cols != plaintext->cols)
        throw std::invalid_argument("Ciphertext and plaintext dimensions do not match!");
    if(commitment->rows != plaintext->rows || commitment->cols != 1)
        throw std::invalid_argument("Invalid dimensions for commitment matrix!");
    if(this->pk.cca && cca_ct == NULL){
        throw std::invalid_argument("Need CCA commitment!");
    }

    std::cout << "\n======== ENCRYPTING ========\n";

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
        threads[i] = std::thread(encrypt_util, std::ref(*this), std::ref(ciphertext), std::ref(commitment), std::ref(cca_ct), std::ref(plaintext), state, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();

    std::cout << "=> " << ciphertext->rows << "x" << ciphertext->cols << " Ciphertext Matrix and Commitment Value Made\n";
}
