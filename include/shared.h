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

#define ENCRYPT 9
#define NO_ENCRYPT 10

#define SCHEDULED 11
#define COMPLETED 12
#define IDLE 13
#define ERROR -1

#define BASE 16


#include <mutex>
#include <condition_variable>
#include "../app/matrix.h"
# include "../tools/sgx_tgmp.h"
// #ifdef HAVE_SGX
// 	# include <sgx_tgmp.h>
// #else
// 	# include <gmp.h>
// #endif
// #include <gmpxx.h>


typedef struct wrapper
{
    float alpha;
    float learning_rate;
    Matrix training_error;
    int update_rows;
    int update_cols;
    int start_idx;
    int batch_size;
    char* sfk;
}*Wrapper;

typedef struct request
{
    int job_id;
    int key_id;
    volatile int status;
    // std::mutex gaurd;
    // std::condition_variable status;

    int tid;
    int num_threads;

    // General input matrix
    Matrix input_1;

    // Compression matrix if present
    Matrix compression;

    // Commitment matrix if present
    Matrix cmt;

    // General output matrix
    Matrix output;

    char* p;
    char* g;
    char* final_sfk;
    int limit;

    Wrapper wp;

}*Request;

Wrapper init_wrapper(float alpha, float learning_rate);
Request init_request(int job_id, int num_threads);
Wrapper* make_wrapper_copy(Wrapper wp, int copies = 1);
Request* make_request_copy(Request req, int copies = 1);