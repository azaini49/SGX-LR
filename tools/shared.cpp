#include "../include/shared.h"
#include "../app/matrix.h"
#include <string.h>

Wrapper init_wrapper(float alpha, float learning_rate)
{
    Wrapper wp = (Wrapper)malloc(sizeof(Matrix*) + 2*sizeof(float) + 4*sizeof(int));
    wp->alpha = alpha;
    wp->learning_rate = learning_rate;

    wp->batch_size = 1024;
    wp->start_idx = 0;
    wp->update_cols = 0;
    wp->update_rows = 0;
    wp->training_error = NULL;

    return wp;
}

Request init_request(int job_id, int num_threads)
{
    Request req = (Request)malloc(4 * sizeof(Matrix*) + 5*sizeof(int) + 3*sizeof(char*) + sizeof(Wrapper));
    req->job_id = job_id;
    req->num_threads = num_threads;
    req->status = SCHEDULED;

    req->g = NULL;
    req->p = NULL;
    req->final_sfk = NULL;
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
        {
            int g_len = strlen(req->g);
            copy->g = (char*)malloc(g_len + 1);
            memcpy(copy->g, req->g, g_len);
            copy->g[g_len] = '\0';
        }
        if(req->p != NULL)
        {
            int p_len = strlen(req->p);
            copy->p = (char*)malloc(p_len + 1);
            memcpy(copy->p, req->p, p_len);
            copy->p[p_len] = '\0';
        }
         copy_arr[i] = copy;
    }
    copy_arr[copies] = NULL;
    return copy_arr;
}