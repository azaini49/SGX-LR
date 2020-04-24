#include "context.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * Default private constructor to initialize the context object
 * @params : security_level(bits), secret key length, field prime, field generator
 * @return : None
 */
Context::Context(int security_level, mpz_t prime, mpz_t gen)
    :security_level(security_level)
{
    if(security_level < 128)
    {
        throw "Illegal parameters for context initialization.";
    }

    // Initialize the FEO object
    mpz_init(this->p);
    mpz_init(this->g);

    // Generate prime
    if(prime == NULL)
    {
        mpz_set_si(this->p, 1);
        generate_safe_prime(security_level);
    }
    else
        mpz_set(this->p, prime);
        
    // Initialize generator
    if(gen == NULL)
        generator();
    else
        mpz_set(this->g, gen);
}

/*
 * Create new context object
 * @params : security_level(bits), secret key length, field prime, field generator
 * @return : Shared pointer to context object
 */
std::shared_ptr<Context> Context::Create(int security_level, mpz_t prime, mpz_t gen)
{
    return std::shared_ptr<Context>(new Context(security_level, prime, gen));
}

/*
 * Generate a safe prime of the specified bit size
 * @params : bit size of the prime
 * @return : safe prime
 */
void Context::generate_safe_prime(int bits)
{
    mpz_t num;
    mpz_init(num);
    mpz_set_ui(num, 1 << (bits-1));

    mpz_t q;
    mpz_init(q);
    while(true)
    {
        mpz_nextprime(q, num);
        mpz_addmul_ui(this->p, q, 2);
        
        if(mpz_probab_prime_p(this->p, 30) > 0)
            break;
        mpz_set_si(this->p, 1);
        mpz_set(num, q);
    }
    mpz_clear(q);
    mpz_clear(num);
}

/*
 * Get the field generator
 * @return : generator
 */
void Context::generator()
{
    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    // Variable to store temporary result
    mpz_t tmp;
    mpz_init(tmp);

    // Compute q such that p = 2q + 1
    mpz_t q;
    mpz_init(q);
    mpz_sub_ui(q, this->p, 1);
    mpz_div_ui(q, q, 2);

    // Store intermediate result
    mpz_t rem;
    mpz_init(rem);

    // Inverse of g modulo p
    mpz_t ginv;
    mpz_init(ginv);

    while(true)
    {
        int safe = 1;
        mpz_urandomm(this->g, state, this->p);
        mpz_add_ui(this->g, this->g, 2);
        mpz_mod(this->g, this->g, this->p);
        mpz_powm_ui(tmp, g, 2, this->p);

        if(mpz_cmp_si(tmp, 1) == 0)
            safe = 0;
        mpz_powm(tmp, this->g, q, this->p);
        if(safe == 1 && mpz_cmp_si(tmp, 1) == 0)
            safe = 0;

        mpz_sub_ui(tmp, this->p, 1);
        mpz_fdiv_r(rem, tmp, this->g);
        
        if(safe == 1 && mpz_cmp_ui(rem, 0) == 0)
            safe = 0;

        mpz_invert(ginv, this->g, this->p);
        mpz_fdiv_r(rem, tmp, ginv);

        if(safe == 1 && mpz_cmp_si(rem, 0) == 0)
            safe = 0;
        if(safe == 1)
            break;  
    }
    // Clear temporary variables
    mpz_clear(q);
    mpz_clear(tmp);
    mpz_clear(rem);
    mpz_clear(ginv);
}