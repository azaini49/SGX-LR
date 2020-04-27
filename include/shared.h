#pragma once

#define FALSE 0
#define TRUE 1

#define GENERATE_LOOKUP_TABLE 2
#define FINAL_PREDICTION 3
#define TRAIN_PREDICTION 4
#define WEIGHT_UPDATE 5
#define SET_FE_SECRET_KEY 6
#define GET_PUB_KEY 7
#define SET_SFK 8
#define TEST_DATA_TRANSMISSION 14

#define ENCRYPT 9
#define NO_ENCRYPT 10

#define SCHEDULED 11
#define COMPLETED 12
#define IDLE 13
#define ERROR -1

#define BASE 16



#include "../tools/matrix_shared.h"
#include "../tools/sgx_tgmp.h"
#include "../tools/gmpxx.h"
#include <stdint.h>

typedef struct response
{
    int key_id;
    int start_idx;
    int batch_size;
    int limit;
    
    float alpha;
    float learning_rate;
    
    // General input matrix
    Matrix input;

    // Compression matrix if present
    Matrix compression;

    // Commitment matrix if present
    Matrix cmt;

    // Store final sfk
    char* out_str;

    // General output matrix
    Matrix output;
    mpz_t p;
    mpz_t g;
    mpz_t final_sfk;

}*Response;

typedef struct request
{
    int job_id;
    int key_id;
    volatile int status;
    int start_idx;
    int batch_size;
    int limit;
    mpz_t final_sfk;   
    float alpha;
    float learning_rate;
    char buffer[1000];
    char out[1000];   
}*Request;

Request init_request(int job_id);
void init_response(Response res);
void deserialize_request(Response res, Request req);
Request serialize_request(int job_id, const Matrix input, const Matrix output, const Matrix compression, const Matrix cmt, mpz_class p, mpz_class g, mpz_class final_sfk, char* buff = NULL);
int serialize_matrix(Matrix mat, uint8_t* buff);
