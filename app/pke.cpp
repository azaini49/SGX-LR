#include "pke.h"
#include <thread>
#include <stdexcept>
#include <ctime>
#include <iostream>

/**
 * Constructer for PubKeyEncr
 * @params : length of secret key
 */
PubKeyEncr::PubKeyEncr(int security_level)
     :security_level(security_level)
{
    
    mpz_init(this->N);
    mpz_init(this->g);
    mpz_init(this->p);
    mpz_init(this->q);
    mpz_init(this->lambda);
    mpz_init(this->mu);


    mpz_t pMin1;
    mpz_t qMin1;
    mpz_init(qMin1);
    mpz_init(pMin1);
    

    // N
    mpz_init_set_si(this->p, 1); //1);
    mpz_init_set_si(this->q, 1);//1);
    generate_safe_prime(security_level, std::ref(this->p));
    generate_safe_prime(security_level, std::ref(this->q));
    std::cout << "p and q: " << mpz_get_si(this->p) << " " << mpz_get_si(this->q) << "\n";
    mpz_sub_ui(pMin1, this->p, 1);
    mpz_sub_ui(qMin1, this->q, 1);
    mpz_mul(this->N, this->p, this->q);
    mpz_mul(this->lambda, pMin1, qMin1); //may need to change sub to mpz_sub_ui
    mpz_invert(this->mu, this->lambda, this->N);
     //col = col + numThreads;
    //}
    

	// Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(0));
    mpz_t Ndiv2;
    mpz_init(Ndiv2);
    mpz_tdiv_q_ui(Ndiv2, this->N, 2);

    mpz_init_set(this->g, this->N);
    mpz_add_ui(this->g, this->g, 1);
    
    /* int star = 0;
    while(!star){
        mpz_t N2;
        mpz_init(N2);
        mpz_mul(N2, this->N, this->N);
        mpz_urandomm(this->g, state, N2);

      star = 1;

      
      if (mpz_cmp_si(this->g,0) <= 0 || mpz_cmp(this->g, Ndiv2) <= 0){
        star = 0;
      } else {
          mpz_t gcd;
          mpz_init(gcd);
          mpz_gcd(gcd, this->g, this->N);
          if (mpz_cmp_si(gcd,1) != 0){
            star = 0;
          }
          mpz_t val;
          mpz_init(val);
          
          mpz_powm(val, this->g, this->lambda, N2);
          mpz_sub_ui(val, val, 1);
          mpz_tdiv_q(val, val, this->N);
          
          mpz_gcd(gcd, val, this->N);
          if (mpz_cmp_si(gcd,1) != 0){
            star = 0;
          }

          mpz_clear(gcd);
          mpz_clear(val);
          
      }
      mpz_clear(N2);
        
    } */



    mpz_clear(pMin1);
    mpz_clear(qMin1);
    mpz_clear(Ndiv2);
    
    

}

std::shared_ptr<PubKeyEncr> PubKeyEncr::Create(int security_level)
{
    return std::shared_ptr<PubKeyEncr>(new PubKeyEncr(security_level));
}


// Utility function to encrypt data
void PubKeyEncr::encrypt_util(std::shared_ptr<PubKeyEncr> pke, Matrix ciphertext, Matrix plaintext, gmp_randstate_t state, int tid, int numThreads)
{
    int row = tid;

    

    // Generate random nonce - let nonce equal the value r in scheme
    mpz_t nonce;
    mpz_init(nonce);

    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);

    mpz_t N2;
    mpz_init(N2);
    mpz_mul(N2, pke->N, pke->N);

    while(row < plaintext->rows)
    {
        // ENCRYPT
        mpz_urandomm(nonce, state, pke->N);
        mpz_add_ui(nonce, nonce, 2);

        //mpz_set_si(nonce, 3); // TEMP


        for(int col = 0; col < plaintext->cols; col++)
        {
            if (mpz_cmp(mat_element(plaintext, row, col), pke->N) >= 0) {
                throw std::invalid_argument("Plaintext dimensions too large!");
            }   
            mpz_powm(mat_element(ciphertext, row, col), pke->g, mat_element(plaintext, row, col), N2);
	        std::cout << "raw ciphertext: " << mpz_get_si(mat_element(ciphertext, row, col)) << "\n";
	        mpz_powm(tmp, nonce, pke->N, N2);
	        std::cout << "randomness: " << mpz_get_si(tmp) << "\n";
            mpz_mul(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), tmp);
            mpz_mod(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), N2);
        }
        row = row + numThreads;
   }
    mpz_clear(nonce);
    mpz_clear(tmp);
    mpz_clear(N2);
}

/**
 * Encrypt plaintext to get the corresponding ciphertext and commitment
 * @params : plaintext integer
 * @return : ciphertext
 */
void PubKeyEncr::encrypt(std::shared_ptr<PubKeyEncr> pke, Matrix ciphertext, Matrix plaintext)
{
    if(ciphertext->rows != plaintext->rows || ciphertext->cols != plaintext->cols)
        throw std::invalid_argument("Ciphertext and plaintext dimentions do not match!");

    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    //encrypt_util(pke, std::ref(ciphertext), std::ref(plaintext), state, 0, 1);

    // Define threadpool
    int numThreads = plaintext->rows;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(encrypt_util, std::ref(pke), std::ref(ciphertext), std::ref(plaintext), state, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();


}


// Utility function to decrypt data
void PubKeyEncr::decrypt_util(std::shared_ptr<PubKeyEncr> pke, Matrix plaintext, Matrix ciphertext, int tid, int numThreads)
{
    int row = tid;

    mpz_t N2;
    mpz_init(N2);
    mpz_mul(N2, pke->N, pke->N);
    
    // Store intermediate values
    mpz_t tmp;
    mpz_init(tmp);


    while(row < plaintext->rows)
    {
        // DeCRYPT
      //  mpz_set_si(nonce, 4); // TEMP


        
        for(int col = 0; col < plaintext->cols; col++)
        {
	
    	
        mpz_powm(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), pke->lambda, N2);
        mpz_tdiv_q(mat_element(ciphertext, row, col), mat_element(ciphertext, row, col), pke->N);
        mpz_mul(mat_element(plaintext, row, col), mat_element(ciphertext, row, col), pke->mu);
        mpz_mod(mat_element(plaintext, row, col), mat_element(plaintext, row, col), pke->N);

        /* (ciphertext, ciphertext, 1);
        
	    mpz_powm(tmp, pke->g, pke->lambda, N2);
	    std::cout << "Tmp init: " << mpz_get_si(tmp) << "\n";
        mpz_sub_ui(tmp, tmp, 1);
	    std::cout << "Tmp after sub by 1: " << mpz_get_si(tmp) << "\n";
         mpz_tdiv_q(tmp, tmp, pke->N);
	    std::cout << "N: " << mpz_get_si(pke->N) << "\n";
	    std::cout << "Tmp after div by N:" << mpz_get_si(tmp) << "\n"; 
	    mpz_tdiv_q(ciphertext, ciphertext, tmp);
	    mpz_mod(plaintext, ciphertext, pke->N); */

        }
        row = row + numThreads;
   }
    mpz_clear(tmp);
    mpz_clear(N2);
}
/**
 * Encrypt plaintext to get the corresponding ciphertext and commitment
 * @params : plaintext integer
 * @return : ciphertext
 */
void PubKeyEncr::decrypt(std::shared_ptr<PubKeyEncr> pke, Matrix plaintext, Matrix ciphertext)
{
    if(ciphertext->rows != plaintext->rows || ciphertext->cols != plaintext->cols)
       throw std::invalid_argument("Ciphertext and plaintext dimentions do not match!");


    // Create random state to use as seed

    //decrypt_util(pke, std::ref(plaintext), std::ref(ciphertext),  0, 1);

    // Define threadpool
    int numThreads = plaintext->rows;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;

    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(encrypt_util, std::ref(pke), std::ref(ciphertext), std::ref(plaintext), i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();


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
    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(0)); 
    mpz_urandomb(num, state, bits);
    std::cout << "Random: " << mpz_get_si(num) << "\n";
    mpz_t security;
    mpz_init(security);
    int val = (1 << (bits -1)) | 1;
    mpz_set_ui(security,val);
    mpz_ior(num, num, security);
    std::cout << "Random num: " << mpz_get_si(num) << "\n";
    mpz_t q;
    mpz_init(q);

int count = 0;
    while(true)
    {
	count++;
        mpz_nextprime(q, num);
        mpz_addmul_ui(prime, q, 2);

        if(mpz_probab_prime_p(prime, 12) > 0)
            break;
        mpz_set_ui(prime, 1);
        mpz_set(num, q);
    }
    std::cout << "num tries to generate prime: " << count << "\n";
    mpz_clear(q);
    mpz_clear(num);
    gmp_randclear(state);
}
