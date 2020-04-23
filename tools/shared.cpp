#include "../include/shared.h"
#include "../app/matrix.h"

Wrapper init_wrapper(float alpha, float learning_rate)
{
    Wrapper wp = (Wrapper)malloc(sizeof(Matrix*) + 2*sizeof(float) + 4*sizeof(int) + sizeof(mpz_t));
    wp->alpha = alpha;
    wp->learning_rate = learning_rate;
    mpz_init(wp->sfk);

    wp->batch_size = 1024;
    wp->start_idx = 0;
    wp->update_cols = 0;
    wp->update_rows = 0;
    wp->training_error = NULL;

    return wp;
}

Request init_request(int job_id, int num_threads)
{
    Request req = (Request)malloc(4 * sizeof(Matrix*) + 4*sizeof(int) + 3*sizeof(mpz_t) + sizeof(mpz_class) + sizeof(Wrapper));
    req->job_id = job_id;
    req->num_threads = num_threads;
    req->status = SCHEDULED;

    mpz_init(req->g);
    mpz_init(req->p);
    mpz_init(req->final_sfk);
    req->wp = NULL;
    req->input_1 = NULL;
    req->cmt = NULL;
    req->compression = NULL;
    req->output = NULL;
    req->limit = 15;
    req->tid = 0;
    req->key_id = -1;

    return req;
}

Wrapper* make_wrapper_copy(Wrapper wp, int copies)
{
    Wrapper* copy_arr = (Wrapper*)malloc(copies * sizeof(Wrapper*) + 2);
    copy_arr[0] = wp;
    for(int i = 1; i < copies + 1; i++)
    {
        Wrapper copy = init_wrapper(wp->alpha, wp->learning_rate);
        copy->batch_size = wp->batch_size;
        copy->start_idx = wp->start_idx;
        copy->update_cols = wp->update_cols;
        copy->update_rows = wp->update_rows;
        copy->training_error = wp->training_error;
        if(wp->sfk != NULL)
            mpz_set(copy->sfk, wp->sfk);
        copy_arr[i] = copy;
    }
    copy_arr[copies] = NULL;
    return copy_arr;
}

Request* make_request_copy(Request req, int copies)
{
    Request* copy_arr = (Request*)malloc(copies * sizeof(Request*) + 2);
    copy_arr[0] = req;
    for(int i = 1; i < copies + 1; i++)
    {
        Request copy = init_request(req->job_id, req->num_threads);
        copy->wp = make_wrapper_copy(req->wp)[0];
        copy->input_1 = req->input_1;
        copy->input_1 = req->input_1;
        copy->output = req->output;
        copy->compression = req->compression;
        copy->cmt = req->cmt;
        if(req->g != NULL)
            mpz_set(copy->g, req->g);
        if(req->p != NULL)
            mpz_set(copy->p, req->p);
         copy_arr[i] = copy;
    }
    copy_arr[copies] = NULL;
    return copy_arr;
}