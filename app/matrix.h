#pragma once

// #include <stdbool.h>
// #include <gmp.h>
#include <vector>
#include <iostream>

// #define mat_element(mat, row_idx, col_idx) mat->data[row_idx * (mat->cols) + col_idx]
// #define mat_element2(mat, row_idx, col_idx) mat.data[row_idx * (mat.cols) + col_idx]

// class Matrix
// {           
//     public:
//         int rows;
//         int cols;
//         mpz_t* data;
//         Matrix(int rows, int cols);
//         Matrix(std::vector<mpz_t> mat);
//         Matrix(std::vector<std::vector<mpz_t> > mat);
//         Matrix(const Matrix &mat);
//         void init(int rows, int cols);
//         ~Matrix();
//         void copy(const Matrix &other);
//         static void generate_random_matrix(Matrix &A, int m);
//         void get(mpz_t result, int row, int col);
//         void set( mpz_t val, int row, int col);
//         void set_row(int row, std::vector<mpz_t> vec);
//         void transpose(Matrix &dest, int first_row = 0, int last_row = -1, int first_col = 0, int last_col = -1);
//         void add_rows(int row1, int row2, int first_col = 0, int last_col = -1);
//         void add_cols(int col1, int col2, int first_row = 0, int last_row = -1);
//         static void add(Matrix &dest, Matrix &A, Matrix &B, mpz_class mod = -1);
//         void swap_row(int row1, int row2);
//         static void multiply(Matrix &dest, Matrix &A, Matrix &B, mpz_class mod = -1);
//         void make_identity();
//         bool is_identity();
//         int is_zero_matrix();
//         bool equals(Matrix &A, int first_row = 0, int last_row = -1, int first_col = 0, int last_col = -1);
//         void inverse();
//         static void splice(Matrix &dest, Matrix &src, int first_row_s, int last_row_s, int first_col_s, int last_col_s, int first_row_d = 0, int last_row_d = -1, int first_col_d = 0, int last_col_d = -1);
//         void concat_horizontal(Matrix &dest, Matrix &B);
//         void concat_vertical(Matrix &dest, Matrix &B);
//         void mod(Matrix &dest, mpz_t mod);
//         static void row_inner_product(mpz_t result, const Matrix &A, const Matrix &B, mpz_class mod = -1, int rowIdx_A = 0, int rowIdx_B = -1, int colBegin_A = 0, int colEnd_A = -1, int colBegin_B = 0, int colEnd_B = -1);
//         static void inner_product(Matrix& dest, Matrix& A, Matrix& B, mpz_class mod = -1);
//         void print(int first_row = 0, int last_row = -1, int first_col = 0, int last_col = -1);

//     private:
//         Matrix();
//     friend class Secret_Key;
//     friend class Public_Key;
//     friend class Logistic_Regression;
// };

#include <stdbool.h>
#include "../tools/sgx_tgmp.h"
#include "../tools/matrix_shared.h"

void setMatrix(Matrix A, mpz_t x);
void add_rows(Matrix A,int row1, int row2);
Matrix add_rows_new(Matrix A,int row1, int row2, int i1, int i2);
Matrix add_cols(Matrix A,int col1, int col2, int a, int b);
void add_matrix(Matrix C, Matrix A, Matrix B, mpz_t mod);
void swap(Matrix A, int row1, int row2);
void matrix_mult(Matrix C, Matrix A, Matrix B, int mod = -1);
void row_major_multiplication(Matrix result, Matrix A, Matrix B, int mod = -1);
void make_indentity(Matrix A);
bool is_identity(Matrix A);
int is_zero_matrix(Matrix A);
bool mat_is_equal(Matrix A, Matrix B, int row1 = 0, int row2 = -1, int col1 = 0, int col2 = -1);
Matrix matrix_inverse(Matrix A);
void mat_splice(Matrix &dest, Matrix &src, int first_row_s, int last_row_s, int first_col_s, int last_col_s, int first_row_d = 0, int last_row_d = -1, int first_col_d = 0, int last_col_d = -1);
Matrix concat_horizontal(Matrix A, Matrix B);
Matrix concat_vertical(Matrix A, Matrix B);
void matrix_mod(Matrix res, Matrix A, mpz_t mod);
void inner_product(Matrix& result, Matrix& A, Matrix& B, int mod = -1);
void print_matrix(Matrix A, int r1 = 0, int r2 = -1, int c1 = 0, int c2 = -1);