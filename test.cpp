#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <gmp.h>
#include <gmpxx.h>

using namespace std;


typedef struct matrix
{
   int rows;             //number of rows.
   int cols;             //number of columns
   mpz_t *data = NULL;
}*Matrix;

typedef struct e_matrix : matrix
{
   mpz_t data[5000];
}*E_Matrix;

#define mat_element(mat, row_idx, col_idx) mat->data[row_idx * (mat->cols) + col_idx]

//initialize the matrix
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
      mpz_init(mat_element(A, i, j));
      mpz_set_si(mat_element(A, i, j), 0);
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

void set_matrix_element(Matrix A, int row_idx, int col_idx, mpz_class val)
{
  if(row_idx < 0 || row_idx >= A->rows || col_idx < 0 || col_idx >= A->cols)
  {
    printf("Matrix index out of range\n");
    return;
    //exit(1);
  }
  //mpz_init(A->data[row_idx * (A->cols) + col_idx]);
  mpz_set(A->data[row_idx * (A->cols) + col_idx], val.get_mpz_t());
}

void simple_test(Matrix mat)
{
    for(int i = 0; i < mat->rows ; i++)
    {
        for(int j = 0; j < mat->cols; j++)
            cout << mat_element(mat, i, j) << " "; 
        cout << "\n";
    }
    cout << "\n";
}



int main(int argc, char const *argv[])
{
    Matrix a;
    a = mat_init(1, 3);
    cout << "a initialized\n";
    mpz_class x{1};
    set_matrix_element(a, 0,0, x);
    set_matrix_element(a, 0,1, x);
    set_matrix_element(a, 0,2, x);

    struct e_matrix b;
    setup_matrix(&b, 1, 3);
    set_matrix_element(&b, 0,0, x);
    set_matrix_element(&b, 0,1, x);
    set_matrix_element(&b, 0,2, x);

    simple_test(a);
    simple_test(a);
}
