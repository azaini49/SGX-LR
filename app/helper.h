#pragma once

# include "../tools/sgx_tgmp.h"
#include <gmpxx.h>
#include <map>
#include <vector>
#include "evaluator.h"
#include "matrix.h"
#include "../include/shared.h"
#include <chrono>

struct Timer
{
    std::string funcName;
    std::chrono::time_point<std::chrono::high_resolution_clock>  start, end;
    Timer(std::string funcName)
        : funcName(funcName)
    {
        start = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
        end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::cout << funcName << " : " << duration << " microseconds\n";
    }
};

void make_request(Request req);
void compute_hamming_distance(Matrix res, const Matrix A, const Matrix B, int row = 0, int A_c1 = 0, int A_c2 = -1, int B_c1 = 0, int B_c2 = -1);
void compute_vector_difference(Matrix res, const Matrix A, const Matrix B, int row = 0, int A_c1 = 0, int A_c2 = -1, int B_c1 = 0, int B_c2 = -1);
void readFile(std::string filename, std::vector<std::vector<int> > &contents);
void populate(Matrix inp, std::vector<std::vector<int> > xtest);
Matrix deserialize_matrix(uint8_t* buff);
