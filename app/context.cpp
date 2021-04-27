#include "context.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <ctime>

/*
 * Default private constructor to initialize the context object
 * @params : security_level(bits), secret key length, field prime, field generator
 * @return : None
 */
Context::Context(int security_level, int Mx, int My)
    :security_level(security_level)
{
    if(security_level < 128)
    {
        throw "Illegal parameters for context initialization.";
    }

    std::cout << "\n==== GENERATING CONTEXT ====\n";

    // Initialize the FEO object
    mpz_init(this->N);
    mpz_init(this->g);
    mpz_init(this->s);
    this->Mx = Mx;
    this->My = My;
    int Mt = 2*My*Mx + 1;

    mpz_set_si(this->s, 2); //s is int >=1
    int s_int = mpz_get_si(this->s);

    // N
    mpz_t p;
    mpz_t q;
    mpz_init_set_si(p, 4079);
    mpz_init_set_si(q, 179);
   // generate_safe_prime(security_level, std::ref(p));
   // generate_safe_prime(security_level, std::ref(q));


    mpz_mul(this->N,p,q);

    // Ns
    mpz_init(this->Ns);
    for (int i = 1; i<s_int; i++){
      mpz_mul(this->Ns, this->N, this->N);
    }

    std::cout << "=> Modulous = " << mpz_get_si(this->Ns) << "\n";

    generator();

    std::cout << "=> Generator = " << mpz_get_si(this->g) << "\n";

    if (mpz_cmp_si(this->Ns, Mt) < 0){
      throw "Message space too Large for N^s.";
    }
}

/*
 * Create new context object
 * @params : security_level(bits), secret key length, field prime, field generator
 * @return : Shared pointer to context object
 */
std::shared_ptr<Context> Context::Create(int security_level, int Mx, int My)
{
    return std::shared_ptr<Context>(new Context(security_level, Mx, My));
}

/*
 * Generate a safe prime of the specified bit size
 * @params : bit size of the prime
 * @return : safe prime
 */
void Context::generate_safe_prime(int bits, mpz_t prime)
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

/*
 * Get the field generator
 * @return : generator
 */
void Context::generator()
{
    // Create random state to use as seed
    gmp_randstate_t state;
    gmp_randinit_mt(state);

    gmp_randseed_ui(state, time(0));

    // U = Z*_{N^{s+1}}
    mpz_t u;
    mpz_init(u);

    // get u in Z*_{N^{s+1}}
    int star = 0;
    while(!star){
      mpz_urandomm(u, state, this->Ns);
      star = 1;

      if (mpz_cmp_si(u,0) <= 0){
        star = 0;
      } else {
          mpz_t gcd;
          mpz_init(gcd);
          mpz_gcd(gcd, u, this->Ns);

          if (mpz_cmp_si(gcd,1) != 0){
            star = 0;
          }
      }
    }


    // TEMP
    //mpz_set_si(u, 17);

    mpz_powm(this->g, u, this->N, this->Ns);

    // Clear temporary variables
    mpz_clear(u);
}
