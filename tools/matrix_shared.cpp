#include "matrix_shared.h"
#include <string.h>
#include <stdlib.h>

extern int SECURITY_BITS;

//initialize the matrix
Matrix mat_init(int rows, int cols)
{
  if(rows <= 0 || cols <= 0)
  {
    return NULL;
  }
  Matrix A;
  A = (Matrix)malloc(sizeof(Matrix) + (rows*cols)*sizeof(mpz_t));
  A->cols = cols;
  A->rows = rows; 
  A->data = (mpz_t *)malloc(rows*cols*sizeof(mpz_t)); 
  for(int i = 0; i < rows; i++)
  {
    for(int j = 0; j < cols; j++)
    {
      mpz_init2(mat_element(A, i, j), SECURITY_BITS);
    }
  }
  return A;
}

void setup_matrix(E_Matrix mat, int rows, int cols)
{
  mat->rows = rows;
  mat->cols = cols;
  for(int i = 0; i < rows*cols; i++)
    mpz_init(mat->data[i]);
}


//Return the matrix element at position given by the indices
void get_matrix_element(mpz_t result, Matrix mat, int row_idx, int col_idx)
{
  if(row_idx < 0 || row_idx >= mat->rows || col_idx < 0 || col_idx >= mat->cols)
  {
    return;
  }
  mpz_set(result, mat_element(mat, row_idx, col_idx));
}

//Set the value of matix element at position given by the indices to "val"
void set_matrix_element(Matrix A, int row_idx, int col_idx, mpz_t val)
{
  if(row_idx < 0 || row_idx >= A->rows || col_idx < 0 || col_idx >= A->cols)
  {
    //printf("Matrix index out of range\n");
    return;
    //exit(1);
  }
  // mpz_init(A->data[row_idx * (A->cols) + col_idx]);
  mpz_set(A->data[row_idx * (A->cols) + col_idx], val);
}

//Set the value of matix element at position given by the indices to "val"
void set_matrix_element_e(E_Matrix A, int row_idx, int col_idx, mpz_t val)
{
  if(row_idx < 0 || row_idx >= A->rows || col_idx < 0 || col_idx >= A->cols)
  {
    //printf("Matrix index out of range\n");
    return;
    //exit(1);
  }
  // mpz_init(A->data[row_idx * (A->cols) + col_idx]);
  mpz_set(A->data[row_idx * (A->cols) + col_idx], val);
}

//Set the indicated row of the matrix A equal to the vector vec
void set_matrix_row(Matrix A, int row, mpz_t* vec)
{
  if(row < 0 || row >= A->rows)
  {
    //printf("Row index out of range\n");
    return;
    //exit(1);
  }
  for(int i = 0; i < A->cols; i++)
  {
    set_matrix_element(A, row, i, vec[i]);
  }
}

//Delete the matrix and free the space in memory
void delete_matrix(Matrix A)
{
  for(int i = 0; i < A->rows; i++)
  {
    for(int j = 0; j < A->cols; j++)
      mpz_clear(mat_element(A, i, j));
  }
  free(A);
}

//Set B as transpose of A
int transpose(Matrix B, Matrix A, int row1, int row2, int col1, int col2)
{
  if(row2 == -1)
    row2 = A->rows - 1;
  if(col2 == -1)
    col2 = A->cols - 1;

  if(B->rows != (col2 - col1 + 1) || B->cols != (row2 - row1 + 1))
  {
    return -1;
  }

  for(int i = row1; i < row2 + 1; i++)
  {
    for(int j = col1; j < col2 + 1; j++)
    {
      set_matrix_element(B, j - col1, i - row1, mat_element(A, i, j));
    }
  }
}

//Set B as transpose of A
int transpose_e(E_Matrix B, E_Matrix A, int row1, int row2, int col1, int col2)
{
  if(row2 == -1)
    row2 = A->rows - 1;
  if(col2 == -1)
    col2 = A->cols - 1;

  if(B->rows != (col2 - col1 + 1) || B->cols != (row2 - row1 + 1))
  {
    return -1;
  }

  for(int i = row1; i < row2 + 1; i++)
  {
    for(int j = col1; j < col2 + 1; j++)
    {
      set_matrix_element_e(B, j - col1, i - row1, mat_element(A, i, j));
    }
  }
}

// Copy elements of A to B
void mat_copy(Matrix B, const Matrix A)
{
  memcpy(B->data, A->data, (A->rows)*(A->cols)*(sizeof(mpz_t)));
}

// Inner product of specified rows in the given matrix
int row_inner_product(mpz_t result, Matrix const A, Matrix const B, int mod, int rowIdx_A, int rowIdx_B, int colBegin_A, int colEnd_A, int colBegin_B, int colEnd_B)
{
    if(rowIdx_B == -1)
        rowIdx_B = rowIdx_A;
        
    if(colEnd_A == -1)
        colEnd_A = A->cols - 1;

    if(colEnd_B == -1)
        colEnd_B = B->cols - 1;

     if(colEnd_A - colBegin_A != colEnd_B - colBegin_B)
    {
        return -1;
    }

    // Check if specified row is valid
    if(rowIdx_A < 0 || rowIdx_A >= A->rows || rowIdx_B >= B->rows || rowIdx_B < 0)
    {
        return -1;
    }

    // Check if specified column range is valid
    if(colBegin_A < 0 || colEnd_A < 0 || colBegin_A > colEnd_A || colEnd_A >= A->cols)
    {
        return -1;
    }

    if(colBegin_B < 0 || colEnd_B < 0 || colBegin_B > colEnd_B || colEnd_B >= B->cols)
    {
        return -1;
    }

    // Perform inner product
    mpz_t tmp;
    mpz_init(tmp);
    mpz_t tmp2;
    mpz_init(tmp2);
    mpz_set_si(tmp2, 0);

    mpz_t mod_;
    mpz_init(mod_);
    mpz_set_si(mod_, mod);

    int range = colEnd_A - colBegin_A + 1;
    for(int i = 0; i < range; i++)
    {
        mpz_mul(tmp, mat_element(A, rowIdx_A, colBegin_A + i), mat_element(B, rowIdx_B, colBegin_B + i));
        mpz_add(tmp2, tmp2, tmp);
    }
    mpz_set(result, tmp2);
    if(mod != -1)
        mpz_mod(result, result, mod_);
    mpz_clear(mod_);
}
