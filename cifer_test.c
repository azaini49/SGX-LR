#include <stdlib.h>
#include <gmp.h>
#include <stdbool.h>
#include <stdio.h>

static const double cfe_sigma_cdt = 0.84932180028801904272150283410;

typedef struct cfe_normal_double_constant {
    mpz_t k;
    mpf_t k_square_inv;
    mpz_t twice_k;
} cfe_normal_double_constant;

typedef struct cfe_vec {
    mpz_t *vec; /** A pointer to the first integer */
    size_t size; /** The size of the vector */
} cfe_vec;

typedef struct cfe_paillier {
    size_t l;
    mpz_t n;
    mpz_t n_square;
    mpz_t bound_x;
    mpz_t bound_y;
    mpf_t sigma;
    mpz_t k_sigma;
    size_t lambda;
    mpz_t g;
} cfe_paillier;


u_int64_t small_primes[15] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53};
u_int64_t small_primes_product = 16294579238595022365u;

void *cfe_malloc(size_t size) {
    void *ptr = malloc(size);

    if (size != 0 && ptr == NULL) {
        perror("Failed to allocate memory");
        abort();
    }

    return ptr;
}

void cfe_vec_dot(mpz_t res, cfe_vec *v1, cfe_vec *v2) {

    mpz_t prod;
    mpz_init(prod);

    // set it to 0, in case it already holds some value != 0
    mpz_set_si(res, 0);

    for (size_t i = 0; i < v1->size; i++) {
        mpz_mul(prod, v1->vec[i], v2->vec[i]);
        mpz_add(res, res, prod);
    }

    mpz_clear(prod);
}

// Finds a prime number of specified bit length.
// If the safe parameter is true, the prime will be a safe prime, e.g a prime p
// where (p-1)/2 is also a prime.
// The prime is assigned to the argument n.
// Returns 0 on success, and some other value != 0 on failure.
// adapted from https://github.com/xlab-si/emmy/blob/master/crypto/common/primes.go
void cfe_get_prime(mpz_t res, size_t bits, bool safe) {
    if (bits < 2) {
        printf("err\n");
    }

    // if we are generating a safe prime, decrease the number of bits by 1
    // as we are actually generating a germain prime and then modifying it to be safe
    // the safe prime will have the correct amount of bits
    size_t n_bits = safe ? bits - 1 : bits;
    size_t n_bytes = (bits + 7) / 8;
    u_int8_t *bytes = (u_int8_t *) cfe_malloc(n_bytes * sizeof(u_int8_t));

    size_t b = n_bits % 8;
    if (b == 0) {
        b = 8;
    }

    mpz_t p, p_safe, big_mod;
    mpz_inits(p, p_safe, big_mod, NULL);


    gmp_randstate_t state;
    gmp_randinit_mt(state);

    while (true) {
        //randombytes_buf(bytes, n_bytes);
            
        int size = n_bytes;

        mpz_t pre;
	mpz_init(pre);
	mpz_urandomb(pre, state, size);	
	mpz_export(bytes, n_bytes, 1,1,0,0,pre);

        bytes[0] &= (u_int8_t) ((1 << b) - 1);

        if (b >= 2) {
            bytes[0] |= 3 << (b - 2);
        } else {
            bytes[0] |= 1;
            if (n_bytes > 1) {
                bytes[1] |= 0x80;
            }
        }

        bytes[n_bytes - 1] |= 1;

        mpz_import(p, n_bytes, 1, 1, 0, 0, bytes);

        u_int64_t mod = mpz_mod_ui(big_mod, p, small_primes_product);

        for (u_int64_t delta = 0; delta < (1 << 20); delta += 2) {
            u_int64_t m = mod + delta;
            bool candidate = true;

            for (size_t i = 0; i < 15; i++) {
                u_int64_t prime = small_primes[i];

                if (m % prime == 0 && (n_bits > 6 || m != prime)) {
                    candidate = false;
                    break;
                }

                if (safe) {
                    u_int64_t m1 = (2 * m + 1) % small_primes_product;

                    if (m1 % prime == 0 && (n_bits > 6 || m1 != prime)) {
                        candidate = false;
                        break;
                    }
                }

            }

            if (candidate) {
                if (delta > 0) {
                    mpz_set_ui(big_mod, delta);
                    mpz_add(p, p, big_mod);
                }

                if (safe) {
                    mpz_mul_ui(p_safe, p, 2);
                    mpz_add_ui(p_safe, p_safe, 1);
                }
                break;
            }
        }

        if (mpz_probab_prime_p(p, 10) && mpz_sizeinbase(p, 2) == n_bits) {
            if (!safe) {
                if (mpz_probab_prime_p(p, 30)) {
                    break;
                }
            } else if (mpz_probab_prime_p(p_safe, 50) && mpz_probab_prime_p(p, 30)) {
                break;
            }
        }
    }

    if (safe) {
        mpz_set(res, p_safe);
    } else {
        mpz_set(res, p);
    }

    free(bytes);
    mpz_clears(p, p_safe, big_mod, NULL);

    if (mpz_sizeinbase(res, 2) != bits) {
        printf("err\n");
    }

    return;
}


void cfe_uniform_sample(mpz_t res, mpz_t upper) {
    // determine the size of buffer to read random bytes in and allocate it
    size_t n_bits = mpz_sizeinbase(upper, 2);
    size_t n_bytes = ((n_bits - 1) / 8) + 1;
    u_int8_t *rand_bytes = (u_int8_t *) cfe_malloc(n_bytes * sizeof(u_int8_t));

    gmp_randstate_t state;
        gmp_randinit_mt(state); 

    while (1) {
        //randombytes_buf(rand_bytes, n_bytes); // get random bytes

        // make a big integer number from random bytes
        // result is always positive
        //mpz_import(res, n_bytes, 1, 1, 0, 0, rand_bytes);
	    
	int size = n_bytes*(2^8);
    	mpz_urandomm(res, state, size);


        // random number too big, divide it
        if (mpz_sizeinbase(res, 2)  > n_bits) {
            mpz_fdiv_r_2exp(res, res, n_bits);
        }
        // if we're below the upper bound, we have a valid random number
        if (mpz_cmp(res, upper) < 0) {
            break;
        }
    }

    free(rand_bytes);
}



// Checks if all coordinates are < bound.
bool cfe_vec_check_bound(cfe_vec *v, mpz_t bound) {
    for (size_t i = 0; i < v->size; i++) {
        if (mpz_cmp(v->vec[i], bound) > 0) {
            return false;
        }
    }

    return true;
}

// Sets res to the i-th element of v.
void cfe_vec_get(mpz_t res, cfe_vec *v, size_t i) {
    mpz_set(res, v->vec[i]);
}

// Sets the i-th element of v to el.
void cfe_vec_set(cfe_vec *v, mpz_t el, size_t i) {
    mpz_set(v->vec[i], el);
}


// Initializes a vector.
void cfe_vec_init(cfe_vec *v, size_t size) {
    v->size = size;
    v->vec = (mpz_t *) cfe_malloc(size * sizeof(mpz_t));

    for (size_t i = 0; i < size; i++) {
        mpz_init(v->vec[i]);
    }
}

void cfe_paillier_init(cfe_paillier *s, size_t l, size_t lambda, size_t bit_len, mpz_t bound_x, mpz_t bound_y) {
    mpz_t p, q, n, n_square, check, g_prime, g, n_to_5, k_sigma;
    mpz_inits(p, q, n, n_square, check, g_prime, g, n_to_5, k_sigma, NULL);

    mpf_t sigma, sigma_cdt, k_sigma_f;
    mpf_inits(sigma, sigma_cdt, k_sigma_f, NULL);

    // generate two safe primes
    cfe_get_prime(p, bit_len, true);
    cfe_get_prime(q, bit_len, true);
    // calculate n = p * q and n^2
    mpz_mul(n, p, q);
    mpz_mul(n_square, n, n);

    // check if the parameters of the scheme are compatible,
    // i.e. security parameter should be big enough that
    // the generated n is much greater than l and the bounds
    mpz_mul(check, bound_x, bound_x);
    mpz_mul_ui(check, check, l);
    if (mpz_cmp(n, check) < 1) {
        
	printf("err\n");    
	//err = CFE_ERR_PARAM_GEN_FAILED;
        //goto cleanup;
    }
    mpz_mul(check, bound_y, bound_y);
    mpz_mul_ui(check, check, l);
    if (mpz_cmp(n, check) < 1) {
        printf("err\n"); 
	//err = CFE_ERR_PARAM_GEN_FAILED;
        //goto cleanup;
    }

    // generate a generator for the 2n-th residues subgroup of Z_n^2*
    cfe_uniform_sample(g_prime, n_square);
    mpz_powm(g, g_prime, n, n_square);
    mpz_powm_ui(g, g, 2, n_square);

    // check if generated g is invertible, which should be the case except with
    // negligible probability
    if (!mpz_invert(check, g, n_square)) {
        printf("err\n"); 
	//err = CFE_ERR_PARAM_GEN_FAILED;
        //goto cleanup;
    }

    // calculate sigma
    mpz_pow_ui(n_to_5, n, 5);
    mpf_set_z(sigma, n_to_5);
    mpf_mul_ui(sigma, sigma, lambda);
    mpf_sqrt(sigma, sigma);
    mpf_add_ui(sigma, sigma, 2);

    // to sample with cfe_normal_double_constant sigma must be
    // a multiple of cfe_sigma_cdt = sqrt(1/(2ln(2))), hence we make
    // it such
    mpf_set_d(sigma_cdt, cfe_sigma_cdt);
    mpf_div(k_sigma_f, sigma, sigma_cdt);
    mpz_set_f(k_sigma, k_sigma_f);
    mpz_add_ui(k_sigma, k_sigma, 1);
    mpf_set_z(k_sigma_f, k_sigma);
    mpf_mul(sigma, k_sigma_f, sigma_cdt);

    // set the parameters for the scheme
    s->l = l;
    mpz_init_set(s->n, n);
    mpz_init_set(s->n_square, n_square);
    mpz_init_set(s->bound_x, bound_x);
    mpz_init_set(s->bound_y, bound_y);
    mpf_init_set(s->sigma, sigma);
    mpz_init_set(s->k_sigma, k_sigma);
    s->lambda = lambda;
    mpz_init_set(s->g, g);

    cleanup:
    mpz_clears(p, q, n, n_square, check, g_prime, g, n_to_5, k_sigma, NULL);
    mpf_clears(sigma, sigma_cdt, k_sigma_f, NULL);
}

void cfe_paillier_free(cfe_paillier *s) {
    mpz_clears(s->n, s->n_square, s->bound_x, s->bound_y, s->g, s->k_sigma, NULL);
    mpf_clear(s->sigma);
}

// res should be uninitialized!
void cfe_paillier_copy(cfe_paillier *res, cfe_paillier *s) {
    res->l = s->l;
    res->lambda = s->lambda;
    mpz_init_set(res->bound_y, s->bound_y);
    mpz_init_set(res->bound_x, s->bound_x);
    mpz_init_set(res->g, s->g);
    mpz_init_set(res->n, s->n);
    mpz_init_set(res->n_square, s->n_square);
    mpf_init_set(res->sigma, s->sigma);
    mpz_init_set(res->k_sigma, s->k_sigma);
}

void cfe_paillier_master_keys_init(cfe_vec *msk, cfe_vec *mpk, cfe_paillier *s) {
    cfe_vec_init(msk, s->l);
     cfe_vec_init(mpk, s->l);
}

void cfe_paillier_generate_master_keys(cfe_vec *msk, cfe_vec *mpk, cfe_paillier *s) {
    if (msk->size != s->l || mpk->size != s->l) {
        printf("err\n");
    }

    mpf_t one;
    mpf_init_set_ui(one, 1);
    //cfe_normal_double_constant sampler;
    //cfe_normal_double_constant_init(&sampler, s->k_sigma);

    mpz_t x, y;
    mpz_inits(x, y, NULL);

    //cfe_normal_double_constant_sample_vec(msk, &sampler);
    //TODO


    for (size_t i = 0; i < s->l; i++) {
        cfe_vec_get(y, msk, i);
        mpz_powm(x, s->g, y, s->n_square);
        cfe_vec_set(mpk, x, i);
    }

    mpf_clear(one);
    mpz_clears(x, y, NULL);
    return;
}

void cfe_paillier_derive_fe_key(mpz_t fe_key, cfe_paillier *s, cfe_vec *msk, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound_y)) {
        printf("err\n");
    }

    cfe_vec_dot(fe_key, msk, y);
    return;
}

void cfe_paillier_ciphertext_init(cfe_vec *ciphertext, cfe_paillier *s) {
    cfe_vec_init(ciphertext, s->l + 1);
}

void cfe_paillier_encrypt(cfe_vec *ciphertext, cfe_paillier *s, cfe_vec *x, cfe_vec *mpk) {
    if (!cfe_vec_check_bound(x, s->bound_x)) {
        printf("err\n");
    }

    mpz_t n_div_4, r, c_0, c_i, x_i, mpk_i, tmp1, tmp2;
    mpz_inits(n_div_4, r, c_0, c_i, x_i, mpk_i, tmp1, tmp2, NULL);
    mpz_div_ui(n_div_4, s->n, 4);
    cfe_uniform_sample(r, n_div_4);

    mpz_powm(c_0, s->g, r, s->n_square);
    cfe_vec_set(ciphertext, c_0, 0);

    for (size_t i = 0; i < s->l; i++) {
        cfe_vec_get(x_i, x, i);
        mpz_mul(tmp1, x_i, s->n);
        mpz_add_ui(tmp1, tmp1, 1);
        cfe_vec_get(mpk_i, mpk, i);
        mpz_powm(tmp2, mpk_i, r, s->n_square);
        mpz_mul(c_i, tmp1, tmp2);
        mpz_mod(c_i, c_i, s->n_square);
        cfe_vec_set(ciphertext, c_i, i + 1);
    }

    mpz_clears(n_div_4, r, c_0, c_i, x_i, mpk_i, tmp1, tmp2, NULL);
    return;
}
void cfe_paillier_decrypt(mpz_t res, cfe_paillier *s, cfe_vec *ciphertext, mpz_t key, cfe_vec *y) {
    if (!cfe_vec_check_bound(y, s->bound_y)) {
        printf("err\n");
    }

    mpz_t key_neg, c_0, c_i, y_i, tmp, half_n;
    mpz_inits(key_neg, c_0, c_i, y_i, tmp, half_n, NULL);
    mpz_neg(key_neg, key);
    cfe_vec_get(c_0, ciphertext, 0);
    mpz_powm(res, c_0, key_neg, s->n_square);

    for (size_t i = 1; i < ciphertext->size; i++) {
        cfe_vec_get(c_i, ciphertext, i);
        cfe_vec_get(y_i, y, i - 1);
        mpz_powm(tmp, c_i, y_i, s->n_square);
        mpz_mul(res, res, tmp);
        mpz_mod(res, res, s->n_square);
    }

    mpz_sub_ui(res, res, 1);
    mpz_mod(res, res, s->n_square);
    mpz_div(res, res, s->n);

    mpz_div_ui(half_n, s->n, 2);
    if (mpz_cmp(res, half_n) > 0) {
        mpz_sub(res, res, s->n);
    }

    mpz_clears(key_neg, c_0, c_i, y_i, tmp, half_n, NULL);
    return;
}


void cfe_uniform_sample_vec(cfe_vec *res, mpz_t max) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_uniform_sample(res->vec[i], max);
    }
}

void cfe_uniform_sample_range(mpz_t res, mpz_t min, mpz_t max) {
    mpz_t max_sub_min;
    mpz_init(max_sub_min);
    mpz_sub(max_sub_min, max, min);

    cfe_uniform_sample(res, max_sub_min); // sets res to be from [0, max-min)
    mpz_add(res, res, min);                     // sets res to be from [min, max)

    mpz_clear(max_sub_min);
}

void cfe_uniform_sample_range_vec(cfe_vec *res, mpz_t lower, mpz_t upper) {
    for (size_t i = 0; i < res->size; i++) {
        cfe_uniform_sample_range(res->vec[i], lower, upper);
    }
}


int main(void){
    size_t l = 50;
    size_t lambda = 128;
    size_t bit_len = 512;
    mpz_t bound_x, bound_y, fe_key, xy_check, xy, bound_x_neg, bound_y_neg;
    mpz_init(bound_x);
    mpz_init(bound_y);
    mpz_init(fe_key);
    mpz_init(xy_check);
    mpz_init(xy);
    mpz_init(bound_x_neg);
    mpz_init(bound_y_neg);
    mpz_set_ui(bound_x, 2);
    mpz_pow_ui(bound_x, bound_x, 10);
    mpz_set(bound_y, bound_x);
    mpz_neg(bound_x_neg, bound_x);
    mpz_add_ui(bound_x_neg, bound_x_neg, 1);
    mpz_neg(bound_y_neg, bound_y);
    mpz_add_ui(bound_y_neg, bound_y_neg, 1);

    cfe_paillier s, encryptor;
    cfe_paillier_init(&s, l, lambda, bit_len, bound_x, bound_y);

    cfe_vec msk, mpk, ciphertext, x, y;
    cfe_vec_init(&x,l);
    cfe_vec_init(&y,l);
    
    cfe_uniform_sample_range_vec(&x, bound_x_neg, bound_x);
    cfe_uniform_sample_range_vec(&y, bound_y_neg, bound_y);
    


    cfe_vec_dot(xy_check, &x, &y);

    cfe_paillier_master_keys_init(&msk, &mpk, &s);
    cfe_paillier_generate_master_keys(&msk, &mpk, &s);

    cfe_paillier_derive_fe_key(fe_key, &s, &msk, &y);

    cfe_paillier_copy(&encryptor, &s);
    cfe_paillier_ciphertext_init(&ciphertext, &encryptor);
    cfe_paillier_encrypt(&ciphertext, &encryptor, &x, &mpk);

    cfe_paillier_decrypt(xy, &s, &ciphertext, fe_key, &y);


    return 0;
}
