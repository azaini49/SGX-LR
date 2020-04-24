#pragma once

# include "../tools/sgx_tgmp.h"
// #include <gmp.h>
#include <gmpxx.h>
#include <map>
#include <vector>
#include "evaluator.h"
#include "matrix.h"
#include "../include/shared.h"

void make_request(Request &req);
void compute_hamming_distance(Matrix &res, const Matrix &A, const Matrix &B, int row = 0, int A_c1 = 0, int A_c2 = -1, int B_c1 = 0, int B_c2 = -1);
void compute_vector_difference(Matrix &res, const Matrix &A, const Matrix &B, int row = 0, int A_c1 = 0, int A_c2 = -1, int B_c1 = 0, int B_c2 = -1);
void readFile(std::string filename, std::vector<std::vector<int> > &contents);
void populate(Matrix &inp, std::vector<std::vector<int> > xtest);
