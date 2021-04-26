#include "pke.h"
#include <thread>
#include <stdexcept>

/**
 * Constructer for PubKeyEncr
 * @params : length of secret key
 */
PubKeyEncr::PubKeyEncr(int security_level)
     :security_level(security_level)
{
    generate_key(security_level);
}

std::shared_ptr<PubKeyEncr> PubKeyEncr::Create(int security_level)
{
    return std::shared_ptr<PubKeyEncr>(new PubKeyEncr(security_level));
}


// Generate public and private keys (USE STRONGER RANDOM NUMBER GENERATOR!!!)
void PubKeyEncr::generate_key_util(PubKeyEncr &pke, int security_level, int tid, int numThreads)
{
    //int col = tid;
    //while(col < gen.msk_len)
    //{
    int col = tid;
    mpz_init(this->N);
    mpz_init(this->g);
    mpz_init(this->p);
    mpz_init(this->q);
    mpz_init(this->lambda);

    // N
    mpz_init_set_si(this->p, 1);//167);
    mpz_init_set_si(this->q, 1);//179);
    generate_safe_prime(security_level, std::ref(this->p));
    generate_safe_prime(security_level, std::ref(this->q));
    mpz_mul(this->N, this->p, this->q);
    mpz_lcm(this->lambda, this->p-1, this->q-1); //may need to change sub to mpz_sub_ui
    
     //col = col + numThreads;
    //}
}

// Generate public key
void PubKeyEncr::generate_key(int security_level)
{

    generate_key_util(*this, security_level, 0, 1);
    // Define threadpool
    //int numThreads = std::thread::hardware_concurrency();
    //std::thread threads[numThreads];
    //for(int i = 0; i < numThreads; i++)
    //{
    //    threads[i] = std::thread(generate_key_util, std::ref(*this), security_level, i, numThreads);
    //}
    //for(int i = 0; i < numThreads; i++)
    //    threads[i].join();
}


// Utility function to encrypt data
void PubKeyEncr::encrypt_util(PubKeyEncr &pke, mpz_t ciphertext, mpz_t plaintext, gmp_randstate_t state, int tid, int numThreads)
{
    //int row = tid;

    if (mpz_cmp(plaintext, this->N) >= 0) {
        throw std::invalid_argument("Plaintext dimensions too large!");
    }

    // Generate random nonce - let nonce equal the value r in scheme
    mpz_t nonce;
    mpz_init(nonce);

    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);

    mpz_t N2;
    mpz_init(N2);
    mpz_mul(N2, this->N, this->N);

    //while(row < plaintext->rows)
    //{
        // ENCRYPT
        mpz_urandomm(nonce, state, this->N);
        mpz_add_ui(nonce, nonce, 2);

      //  mpz_set_si(nonce, 4); // TEMP


        // compute cti as (1 + N)^xi times pki^r in group G (assuming p is the modulus for group G)
        //for(int col = 0; col < plaintext->cols; col++)
        //{
            mpz_powm(ciphertext, this->g, plaintext, N2);
            mpz_powm(tmp, nonce, this->N, N2);
            mpz_mul(ciphertext, ciphertext, tmp);

        //}
        //row = row + numThreads;
   //}
    mpz_clear(nonce);
    mpz_clear(tmp);
    mpz_clear(N2);
}

/**
 * Encrypt plaintext to get the corresponding ciphertext and commitment
 * @params : plaintext integer
 * @return : ciphertext
 */
void PubKeyEncr::encrypt(mpz_t ciphertext, mpz_t plaintext)
{
    //if(ciphertext->rows != plaintext->rows || ciphertext->cols != plaintext->cols)
    //    throw std::invalid_argument("Ciphertext and plaintext dimentions do not match!");
    //if(commitment->rows != plaintext->rows || commitment->cols != 1)
    //    throw std::invalid_argument("Invalid dimensions for commitment matrix");

    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    encrypt_util(std::ref(*this), std::ref(ciphertext), std::ref(plaintext), state, 0, 1);

    // Define threadpool
    //int numThreads = plaintext->rows;
    //int numCores = std::thread::hardware_concurrency();
    //if(numThreads > numCores)
    //    numThreads = numCores;

    //std::thread threads[numThreads];
    //for(int i = 0; i < numThreads; i++)
    //{
    //    threads[i] = std::thread(encrypt_util, std::ref(*this), std::ref(ciphertext), std::ref(commitment), std::ref(plaintext), state, i, numThreads);
    //}
    //for(int i = 0; i < numThreads; i++)
    //    threads[i].join();


}


// Utility function to decrypt data
void PubKeyEncr::decrypt_util(PubKeyEncr &pke, mpz_t plaintext, mpz_t ciphertext, int tid, int numThreads)
{
    //int row = tid;

    mpz_t N2;
    mpz_init(N2);
    mpz_mul(N2, this->N, this->N);
    if (mpz_cmp(ciphertext, N2) >= 0) {
        throw std::invalid_argument("Ciphertext dimensions too large!");
    }



    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);


    //while(row < plaintext->rows)
    //{
        // ENCRYPT
      //  mpz_set_si(nonce, 4); // TEMP


        // compute cti as (1 + N)^xi times pki^r in group G (assuming p is the modulus for group G)
        //for(int col = 0; col < plaintext->cols; col++)
        //{
            mpz_powm(ciphertext, ciphertext, this->lambda, N2);
            mpz_sub_ui(ciphertext, ciphertext, 1);
            mpz_tdiv_q(ciphertext, ciphertext, this->N);
            mpz_powm(tmp, this->g, this->lambda, N2);
            mpz_sub_ui(tmp, tmp, 1);
            mpz_tdiv_q(tmp, tmp, this->N);
            mpz_tdiv_q(ciphertext, ciphertext, tmp);
            mpz_mod(plaintext, ciphertext, this->N);

        //}
        //row = row + numThreads;
   //}
    mpz_clear(tmp);
    mpz_clear(N2);
}

/**
 * Encrypt plaintext to get the corresponding ciphertext and commitment
 * @params : plaintext integer
 * @return : ciphertext
 */
void PubKeyEncr::decrypt(mpz_t plaintext, mpz_t ciphertext)
{
    //if(ciphertext->rows != plaintext->rows || ciphertext->cols != plaintext->cols)
    //    throw std::invalid_argument("Ciphertext and plaintext dimentions do not match!");
    //if(commitment->rows != plaintext->rows || commitment->cols != 1)
    //    throw std::invalid_argument("Invalid dimensions for commitment matrix");

    // Create random state to use as seed

    decrypt_util(std::ref(*this), std::ref(plaintext), std::ref(ciphertext),  0, 1);

    // Define threadpool
    //int numThreads = plaintext->rows;
    //int numCores = std::thread::hardware_concurrency();
    //if(numThreads > numCores)
    //    numThreads = numCores;

    //std::thread threads[numThreads];
    //for(int i = 0; i < numThreads; i++)
    //{
    //    threads[i] = std::thread(encrypt_util, std::ref(*this), std::ref(ciphertext), std::ref(commitment), std::ref(plaintext), state, i, numThreads);
    //}
    //for(int i = 0; i < numThreads; i++)
    //    threads[i].join();


}


// Return the secret key
//const Secret_Key& PubKeyEncr::secret_key() const
//{
//    return this->sk;
//}

// Return the public key
//const Public_Key& Keygen::public_key() const
//{

//    return this->pk;
//}

void PubKeyEncr::generate_safe_prime(int bits, mpz_t prime)
{
    mpz_t num;
    mpz_init(num);
    mpz_set_ui(num, 1 << (bits-1));

    mpz_t q;
    mpz_init(q);


    while(true)
    {
        mpz_nextprime(q, num);
        mpz_addmul_ui(prime, q, 2);

        if(mpz_probab_prime_p(prime, 30) > 0)
            break;
        mpz_set_si(prime, 1);
        mpz_set(num, q);
    }
    mpz_clear(q);
    mpz_clear(num);
}
