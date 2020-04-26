#pragma once

#include "sgx_tgmp.h"

#define mat_element(mat, row_idx, col_idx) mat->data[row_idx * (mat->cols) + col_idx]
typedef struct matrix
{
   int rows;             //number of rows.
   int cols;             //number of columns.
   mpz_t *data;
}*Matrix;

Matrix mat_init(int rows, int cols);
void get_matrix_element(mpz_t result, Matrix mat, int row_idx, int col_idx);
void set_matrix_element(Matrix A, int row_idx, int col_idx, mpz_t val);
void set_matrix_row(Matrix A, int row, mpz_t* vec);
void delete_matrix(Matrix A);
int transpose(Matrix B, Matrix A, int row1 = 0, int row2 = -1, int col1 = 0, int col2 = -1);
void mat_copy(Matrix B, const Matrix A);
int row_inner_product(mpz_t result, const Matrix A, const Matrix B, int mod = -1, int rowIdx_A = 0, int rowIdx_B = -1, int colBegin_A = 0, int colEnd_A = -1, int colBegin_B = 0, int colEnd_B = -1);


