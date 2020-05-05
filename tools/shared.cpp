#include "../include/shared.h"
#include "matrix_shared.h"
#include <string.h>
#include <stdlib.h>

Request init_request(int job_id)
{
    Request req = (Request)malloc(sizeof(Request) + 6*sizeof(int) + 2*sizeof(float) + 2000*sizeof(char));
    req->job_id = job_id;
    req->status = SCHEDULED;
    req->limit = 15;
    req->key_id = -1;
    req->start_idx = 0;
    req->batch_size = 0;
    req->alpha = 0;
    req->learning_rate = 0;
    mpz_init(req->final_sfk);
    return req;
}

void init_response(Response res)
{
    res->input = NULL;
    res->cmt = NULL;
    res->compression = NULL;
    res->output = NULL;
    res->limit = 15;
    res->key_id = -1;
    res->start_idx = 0;
    res->batch_size = 0;
    res->alpha = 0;
    res->learning_rate = 0;
    mpz_init(res->final_sfk);
    mpz_init(res->p);
    mpz_init(res->g);

}

Request serialize_request(int job_id, const Matrix input, const Matrix output, const Matrix compression, const Matrix cmt, mpz_class p, mpz_class g, char* buff)
{
    Request req = init_request(job_id);
    int idx = 0;
    if(input == NULL)
    {
        uintptr_t ptr = 0;
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }
    else
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(input);
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }

    if(output == NULL)
    {
        uintptr_t ptr = 0;
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }
    else
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(output);
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }

    if(compression == NULL)
    {
        uintptr_t ptr = 0;
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }
    else
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(compression);
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }

    if(cmt == NULL)
    {
        uintptr_t ptr = 0;
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }
    else
    {
        uintptr_t ptr = reinterpret_cast<uintptr_t>(cmt);
        char* tmp = (char*)&ptr;
        for(int i = 0; i < sizeof(uintptr_t); i++)
            req->buffer[i + idx] = tmp[i];
        idx = idx + sizeof(uintptr_t);
    }


    char* ptr = mpz_get_str(NULL, BASE, p.get_mpz_t());
    uintptr_t upt = reinterpret_cast<uintptr_t>(ptr);
    char* tmp = (char*)&upt;
    for(int i = 0; i < sizeof(uintptr_t); i++)
        req->buffer[i + idx] = tmp[i];
    idx = idx + sizeof(uintptr_t);

    ptr = mpz_get_str(NULL, BASE, g.get_mpz_t());
    upt = reinterpret_cast<uintptr_t>(ptr);
    tmp = (char*)&upt;
    for(int i = 0; i < sizeof(uintptr_t); i++)
        req->buffer[i + idx] = tmp[i];
    idx = idx + sizeof(uintptr_t);

    // ptr = mpz_get_str(NULL, BASE, final_sfk.get_mpz_t());
    // upt = reinterpret_cast<uintptr_t>(ptr);
    // tmp = (char*)&upt;
    // for(int i = 0; i < sizeof(uintptr_t); i++)
    //     req->buffer[i + idx] = tmp[i];
    // idx = idx + sizeof(uintptr_t);

    // if(buff == NULL)
    // {
    //     uintptr_t ptr = 0;
    //     char* tmp = (char*)&ptr;
    //     for(int i = 0; i < sizeof(uintptr_t); i++)
    //         req->buffer[i + idx] = tmp[i];
    //     idx = idx + sizeof(uintptr_t);
    // }
    // else
    // {
    //     uintptr_t ptr = reinterpret_cast<uintptr_t>(buff);
    //     char* tmp = (char*)&ptr;
    //     for(int i = 0; i < sizeof(uintptr_t); i++)
    //         req->buffer[i + idx] = tmp[i];
    //     idx = idx + sizeof(uintptr_t);
    // }

    return req;
}

void deserialize_request(Response res, Request req)
{
    int idx = 0;
    uintptr_t inp_p;
    memcpy(&inp_p, &req->buffer[idx], sizeof(uintptr_t));
    if(inp_p == 0)
        res->input = NULL;
    else
        res->input = reinterpret_cast<Matrix>(inp_p); 
    idx = idx + sizeof(uintptr_t);

    uintptr_t out_p;
    memcpy(&out_p, &req->buffer[idx], sizeof(uintptr_t));
    if(out_p == 0)
        res->output = NULL;
    else
        res->output = reinterpret_cast<Matrix>(out_p); 
    idx = idx + sizeof(uintptr_t);

    uintptr_t comp_p;
    memcpy(&comp_p, &req->buffer[idx], sizeof(uintptr_t));
    if(comp_p == 0)
        res->compression = NULL;
    else
        res->compression = reinterpret_cast<Matrix>(comp_p); 
    idx = idx + sizeof(uintptr_t);

    uintptr_t cmt_p;
    memcpy(&cmt_p, &req->buffer[idx], sizeof(uintptr_t));
    if(cmt_p == 0)
        res->cmt = NULL;
    else
        res->cmt = reinterpret_cast<Matrix>(cmt_p); 
    idx = idx + sizeof(uintptr_t);

    uintptr_t p_p;
    memcpy(&p_p, &req->buffer[idx], sizeof(uintptr_t));
    if(p_p == 0)
        mpz_set_si(res->p, 0);
    else
        mpz_set_str(res->p, reinterpret_cast<char*>(p_p), BASE); 
    idx = idx + sizeof(uintptr_t);

    uintptr_t g_p;
    memcpy(&g_p, &req->buffer[idx], sizeof(uintptr_t));
    if(g_p == 0)
        mpz_set_si(res->g, 0);
    else
        mpz_set_str(res->g, reinterpret_cast<char*>(g_p), BASE); 
    idx = idx + sizeof(uintptr_t);

    // uintptr_t sfk_p;
    // memcpy(&sfk_p, &req->buffer[idx], sizeof(uintptr_t));
    // if(sfk_p == 0)
    //     mpz_set_si(res->final_sfk, 0);
    // else
    //     mpz_set_str(res->final_sfk, reinterpret_cast<char*>(sfk_p), BASE); 
    // idx = idx + sizeof(uintptr_t);

    // uintptr_t buff_p;
    // memcpy(&buff_p, &req->buffer[idx], sizeof(uintptr_t));
    // if(buff_p == 0)
    //     res->out_str = NULL;
    // else
    //     res->out_str = reinterpret_cast<char*>(buff_p); 
    // idx = idx + sizeof(uintptr_t);

    res->limit = req->limit;
    res->key_id = req->key_id;
    res->start_idx = req->start_idx;
    res->batch_size = req->batch_size;
    res->alpha = req->alpha;
    res->learning_rate = req->learning_rate;
}

