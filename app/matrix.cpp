#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"
#include <thread>
#include <iostream>
#include <mutex>
#include <string.h>

std::mutex gaurd2;

// Utility function for setMatrix
void setMatrixUtility(Matrix A, mpz_t x, int tid, int numThreads)
{
  int row = tid;
  while(row < A->rows)
  {
    for(int col = 0; col < A->cols; col++)
      mpz_set(mat_element(A, row, col), x);
    row = row + numThreads;
  }
}

// Set every element of matrix to 'x'
void setMatrix(Matrix A, mpz_t x)
{
  // Define threadpool
  int numThreads = A->rows;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;

  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(setMatrixUtility, A, x, i, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();
}

// Utility function to add elements of two rows
void add_rows_util(Matrix A, int row1, int row2, mpz_t mod, int tid, int numThreads)
{
  int x = tid;
  while(x < A->cols)
  {
    mpz_add(mat_element(A, row2, x), mat_element(A, row1, x), mat_element(A, row2, x));
    if(mpz_cmp_si(mod, -1) != 0)
      mpz_mod(mat_element(A, row2, x), mat_element(A, row2, x), mod);
    x = x + numThreads;
  }
}

// Add row1 to row2 of matrix A
void add_rows(Matrix A, int row1, int row2, mpz_t mod)
{
  if(row1 < 0 || row1 >= A->rows || row2 < 0 || row2 >= A->rows)
  {
    printf("Matrix index out of range\n");
    exit(1);
  }
  
  // Define threadpool
  int numThreads = A->rows;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;

  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(add_rows_util, A, row1, row2, mod, i, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();
}

// Utility function for matrix addition
void add_matrix_util(Matrix C, Matrix A, Matrix B, mpz_t mod, int tid, int numThreads)
{
  int row = tid;
  mpz_t tot;
  mpz_init(tot);
  while(row < A->rows)
  {
    for(int col = 0; col < A->cols; col++)
    {
      mpz_add(tot, mat_element(A, row, col), mat_element(B, row, col));
      if(mpz_cmp_si(mod, -1) != 0)
        mpz_mod(tot, tot, mod);
      set_matrix_element(C, row, col, tot);
      mpz_set_si(tot, 0);
    }
    row = row + numThreads;
  }
  mpz_clear(tot);
}

// Add A and B elementwise and store result in C
void add_matrix(Matrix C, Matrix A, Matrix B, mpz_t mod)
{
  if(A->rows != B->rows || A->cols != B->cols)
  {
    printf("Incompatible dimenions for matrix addition.\n");
    exit(1);
  }

  // Define threadpool
  int numThreads = A->rows;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;

  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(add_matrix_util, C, A, B, mod, i, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();
}


//Function to swap two rows of matrix A
void swap(Matrix A, int row1, int row2)
{
  if(row1 < 0 || row1 >= A->rows || row2 < 0 || row2 >= A->rows)
  {
    printf("Matrix index out of range\n");
    exit(1);
  }
  mpz_t temp;
  mpz_init(temp);
  for(int i = 0; i < A->cols; i++)
  {
    mpz_set(temp, mat_element(A, row1, i));
    mpz_set(mat_element(A, row1, i), mat_element(A, row2, i));
    mpz_set(mat_element(A, row2, i), temp);
  }
  mpz_clear(temp);
}

// Utility function for multithreaded matrix multiplication
void mult_util(Matrix C, Matrix A, Matrix B, int tid, mpz_t mod, int numThreads)
{
  mpz_t tmp;
  mpz_init(tmp);
  mpz_t val;
  mpz_init(val);
  mpz_set_si(val, 0);

  int row = tid;
  while(row < A->rows)
  {
    for(int col = 0; col < B->rows; col++)
    {
      for(int k = 0; k < B->cols; k++)
      {
        mpz_mul(tmp, mat_element(A, row, k), mat_element(B, col, k));
        mpz_add(val, val, tmp);
      }
      if(mpz_cmp_si(mod, -1) != 0)
        mpz_mod(val, val, mod);
      mpz_set(mat_element(C, row, col), val);
      mpz_set_si(val, 0);
    }
    row = row + numThreads;
  }
  mpz_clear(val);
  mpz_clear(tmp);
}

// Multithreaded matrix multiplication
void matrix_mult(Matrix C, Matrix A, Matrix B, int mod)
{
  if(A->cols != B->rows)
  {
    printf("Matrices are incompatible, check dimensions...\n");
    std::cout << "A->cols : " << A->cols << std::endl;
    std::cout << "B->rows : " << B->rows << std::endl;
    exit(1);
  }

  if(B->cols != C->cols || A->rows != C->rows)
  {
    printf("Check dimensions of the result matrix.\n");
    std::cout << "Required dimensions: " << A->rows << " x " << B->cols << std::endl;
    std::cout << "Dimensions Provided: " << C->rows << " x " << C->cols << std::endl;
    exit(1);
  }
  Matrix B_temp = mat_init(B->cols, B->rows);
  transpose(B_temp, B);

  mpz_t mod_;
  mpz_init(mod_);
  mpz_set_si(mod_, mod);

  // Define threadpool
  int numThreads = A->rows;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;
  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(mult_util, C, A, B_temp, i, mod_, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();
  delete_matrix(B_temp);
  mpz_clear(mod_);
}

// Custom multiplication using row-wise multiplication for better performance: Result is A(B^T)
void row_major_multiplication(Matrix result, Matrix A, Matrix B, int mod)
{
  // Verify dimensions of input matrices
  if(A->cols != B->cols)
  {
    printf("Matrices are incompatible, check dimensions...\n");
    std::cout << "A->cols : " << A->cols << std::endl;
    std::cout << "B->cols : " << B->cols << std::endl;
    exit(1);
  }

  // Verify dimensions of result matrix
  if(B->rows != result->cols || A->rows != result->rows)
  {
    printf("Check dimensions of the result matrix.\n");
    std::cout << "Required dimensions: " << A->rows << " x " << B->rows << std::endl;
    std::cout << "Dimensions Provided: " << result->rows << " x " << result->cols << std::endl;
    exit(1);
  }

  mpz_t mod_;
  mpz_init(mod_);
  mpz_set_si(mod_, mod);

  // Define threadpool
  int numThreads = A->rows;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;
  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(mult_util, result, A, B, i, mod_, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();
  mpz_clear(mod_);
}


//Set matrix as identity matrix
void make_indentity(Matrix A)
{
  for(int i = 0; i < A->rows; i++)
  {
    for(int j = 0; j < A->cols; j++)
    {
      if(i == j)
      {
        mpz_set_si(mat_element(A, i, j), 1);
      }
      else
      {
        mpz_set_si(mat_element(A, i, j), 0);
      }
    }
  }
}

bool is_identity(Matrix A)
{
  bool flag = true;
  for(int i = 0; i < A->rows; i++)
  {
    for(int j = 0; j < A->cols; j++)
    {
      if(i == j)
      {
        if(mpz_cmp_si(mat_element(A, i, j), 0) == 0)
        {
          flag = false;
          return flag;
        }
      }
      else
      {
        if(mpz_cmp_si(mat_element(A, i, j), 1) == 0)
        {
          flag = false;
          return flag;
        }
      }
    }
  }
  return flag;
}

//Checks if the matrix is a zero matrix
int is_zero_matrix(Matrix A)
{
  int flag = 1;
  for(int i = 0; i < A->rows; i++)
  {
    for(int j = 0; j < A->cols; j++)
    {
      if(mpz_cmp_ui(mat_element(A, i, j),0) == 0)
      {
        flag = 0;
        return flag;
      }
    }
  }
  return flag;
}

// Utility function to check matrix equality
void equal_util(Matrix A, Matrix B, int* status, int row2, int col1, int col2, int tid, int numThreads)
{
  int row = tid;
  while(row < row2 + 1)
  {
    for(int col = col1; col < col2 + 1; col++)
    {
      if(mpz_cmp(mat_element(A, row, col), mat_element(B, row, col)) != 0)
      {
        gaurd2.lock();
        *status = 1;
        gaurd2.unlock();
        return;
      }
    }
    row = row + numThreads;
  }
}

// Check if two matrices are equal (multithreaded)
bool mat_is_equal(Matrix A, Matrix B, int row1, int row2, int col1, int col2)
{
  if(A->rows != B->rows || A->cols != B->cols)
    return false;

  if(row2 == -1)
    row2 = A->rows - 1;
  if(col2 == -1)
    col2 = A->cols - 1;
  int status = 0;

  // Check the specified range
  if(row1 < 0 || row1 >= A->rows || row2 < 0 || row2 >= A->rows || col1 < 0 || col1 >= A->cols || col2 < 0 || col2 >= A->cols)
  {
    std::cout << "Range specified is invalid.\n";
    std::cout << "Row range should lie within [0, " << A->rows - 1 << "]\n";
    std::cout << "Column range should lie within [0, " << A->cols - 1 << "]\n";
    exit(1);
  }

  // Define threadpool
  int numThreads = row2 - row1 + 1;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;
  std::thread threads[numThreads];
  for(int i = row1; i < numThreads + row1; i++)
  {
    threads[i-row1] = std::thread(equal_util, A, B, &status, row2, col1, col2, i, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();

  if(status == 1)
    return false;
  return true;
}


//Add the elements of row1 to row2 in the column index range [a,b]  
Matrix add_rows_new(Matrix A,int row1, int row2, int a, int b)
{
  if(row1 < 0 || row1 >= A->rows || row2 < 0 || row2 >= A->cols)
  {
    printf("Matrix index out of range\n");
    exit(1);
  }
  for(int i = a; i < b; i++)
  {
    mpz_add(mat_element(A, row2, i), mat_element(A, row1, i), mat_element(A, row2, i));
  }
  return A;
}

//Add col1 and col2 from in the row index range [a,b]
Matrix add_cols(Matrix A,int col1, int col2, int a, int b)
{
  if(col1 < 0 || col1 >= A->cols || col2 < 0 || col2 >= A->cols)
  {
    printf("Matrix index out of range\n");
    exit(1);
  }
  for(int i = a; i < b; i++)
  {
    mpz_add(mat_element(A, i, col2), mat_element(A, i, col1), mat_element(A, i, col2));
  }
  return A;
}


// Obtain the specified number of rows and columns
void mat_splice(Matrix dest, Matrix src, int first_row_s, int last_row_s, int first_col_s, int last_col_s, int first_row_d, int last_row_d, int first_col_d, int last_col_d)
{
    int row_count = last_row_s - first_row_s + 1;
    int col_count = last_col_s - first_col_s + 1;

    if(last_row_d == -1)
    last_row_d = dest->rows - 1;
    if(last_col_d == -1)
    last_col_d = dest->cols - 1;

    if(first_row_s < 0 || first_row_s >= src->rows || last_row_s < 0 || last_row_s >= src->rows)
    {
        std::cout << "Error: Splice range out of bounds.\n";
        std::cout << "Requested Row range: [" << first_row_s << ", " << last_row_s << "] for matrix of dimensions: " << src->rows << " x " << src->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    if(first_col_s < 0 || first_col_s >= src->cols || last_col_s < 0 || last_col_s >= src->cols)
    {
        std::cout << "Error: Splice range out of bounds.\n";
        std::cout << "Requested Column range: [" << first_col_s << ", " << last_col_s << "] for matrix of dimensions: " << src->rows << " x " << src->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    if(first_row_d < 0 || first_row_d >= dest->rows || last_row_d < 0 || last_row_d >= dest->rows)
    {
        std::cout << "Error: Splice range out of bounds.\n";
        std::cout << "Requested Row range: [" << first_row_d << ", " << last_row_d << "] for matrix of dimensions: " << dest->rows << " x " << dest->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    if(first_col_d < 0 || first_col_d >= dest->cols || last_col_d < 0 || last_col_d >= dest->cols)
    {
        std::cout << "Error: Splice range out of bounds.\n";
        std::cout << "Requested Column range: [" << first_col_d << ", " << last_col_d << "] for matrix of dimensions: " << dest->rows << " x " << dest->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    if(last_row_d - first_row_d + 1 != row_count || last_col_d - first_col_d + 1 != col_count)
    {
        std::cout << "Error: Dimensions of result matrix incompatible with splice range\n";
        std::cout << "Dimensions of Result Matrix: " << dest->rows << " x " << dest->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    int idx1, idx2;

    for(int i = first_row_d; i < first_row_d + row_count; i++)
    {
        idx1 = first_row_s + i - first_row_d;
        for(int j = first_col_d; j < first_col_d + col_count; j++)
        {
            idx2 = first_col_s + j - first_col_d;
            set_matrix_element(dest, i, j, mat_element(src, idx1, idx2));
        }
    }
}


//Concatenate the matrices A and B as [A|B]
Matrix concat_horizontal(Matrix A, Matrix B)
{
  if(A->rows != B->rows)
  {
    printf("Incompatible dimensions of the two matrices. Number of rows should be same.\n");
    exit(1);
  }
  Matrix temp = mat_init(A->rows, A->cols + B->cols);
  for(int i = 0; i < temp->rows; i++)
  {
    for(int j = 0; j < temp->cols; j++)
    {
      if(j < A->cols)
      {
        set_matrix_element(temp, i, j, mat_element(A, i, j));
      }
      else
      {
        set_matrix_element(temp, i, j, mat_element(B, i, j - A->cols));
      }
    }
  }
  return temp;
}

//Concatenate the matrices A and B vertically
Matrix concat_vertical(Matrix A, Matrix B)
{
  if(A->cols != B->cols)
  {
    printf("Incompatible dimensions of the two matrices. Number of rows should be same.\n");
    exit(1);
  }
  Matrix temp = mat_init(A->rows + B->rows, A->cols);
  for(int i = 0; i < temp->rows; i++)
  {
    for(int j = 0; j < temp->cols; j++)
    {
      if(i < A->rows)
      {
        set_matrix_element(temp, i, j, mat_element(A, i, j));
      }
      else
      {
        set_matrix_element(temp, i, j, mat_element(B, i - A->rows, j));
      }
    }
  }
  return temp;
}

// Utility function to compute mod
void matrix_mod_util(Matrix res, Matrix A, mpz_t mod, int tid, int numThreads)
{
  int row = tid;
  mpz_t tmp;
  mpz_init(tmp);
  while(row < A->rows)
  {
    for(int i = 0; i < A->cols; i++)
    {
      mpz_mod(tmp, mat_element(A, row, i), mod);
      set_matrix_element(res, row, i, tmp);
    }
    row = row + numThreads;
  }
  mpz_clear(tmp);
}

// Convert every element of matrix modulo 'mod'
void matrix_mod(Matrix res, Matrix A, mpz_t mod)
{
  if(A->rows != res->rows || A->cols != res->cols)
  {
    std::cout << "Error: Dimensions of imput and result matrix do not match.\n";
    exit(1);
  }

  // Define threadpool
  int numThreads = A->rows;
  int numCores = std::thread::hardware_concurrency();
  if(numThreads > numCores)
    numThreads = numCores;
  std::thread threads[numThreads];
  for(int i = 0; i < numThreads; i++)
  {
    threads[i] = std::thread(matrix_mod_util, res, A, mod, i, numThreads);
  }

  for(int i = 0; i < numThreads; i++)
    threads[i].join();
}

void inner_product_util(Matrix result, Matrix A, Matrix B, int mod, int tid, int numThreads)
{
    int row = tid;
    while(row < A->rows)
    {
        row_inner_product(mat_element(result, row, 0), A, B, mod, row);
        row = row + numThreads;
    }
}

// Row-wise inner product of two matrices
void inner_product(Matrix result, Matrix A, Matrix B, int mod)
{
    // Check dimensions of input matrices
    if(A->rows != B->rows || A->cols != B->cols)
    {
        std::cout << "[IP] Dimensions of input matrices do not match.\n";
        std::cout << "A : " << A->rows << " x " << A->cols << std::endl;
        std::cout << "B : " << B->rows << " x " << B->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    // Check dimensions of result matrix
    if(A->rows != result->rows || result->cols != 1)
    {
        std::cout << "[IP] Dimensions of input matrices do not match.\n";
        std::cout << "Expected : " << A->rows << " x " << A->cols << std::endl;
        std::cout << "Received : " << result->rows << " x " << result->cols << std::endl;
        exit(EXIT_FAILURE);
    }

    // Define threadpool
    int numThreads = A->rows;
    int numCores = std::thread::hardware_concurrency();
    if(numThreads > numCores)
        numThreads = numCores;
    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
        threads[i] = std::thread(inner_product_util,result, A, B, mod, i, numThreads);

    for(int i = 0; i < numThreads; i++)
        threads[i].join();
}


//Printing the matrix
void print_matrix(Matrix A, int r1, int r2, int c1, int c2)
{
  if(r2 == -1)
    r2 = A->rows - 1;
  if(c2 == -1)
    c2 = A->cols - 1;
  
  if(r1 < 0 || r2 < 0 || r1 >= A->rows || r2 >= A->rows || r1 > r2)
  {
    std::cout << "Invalid Row range\n";
    std::cout << "Row indices expected in range : [0, " << A->rows - 1 << "]\n";
    std::cout << "Range provided : [" << r1 << ", " << r2 << "]\n";
    exit(1); 
  }
  if(c1 < 0 || c2 < 0 || c1 >= A->cols || c2 >= A->cols || c1 > c2)
  {
    std::cout << "Invalid Column range\n";
    std::cout << "Column indices expected in range : [0, " << A->cols - 1 << "]\n";
    std::cout << "Range provided : [" << c1 << ", " << c2 << "]\n";
    exit(1); 
  }
  for(int i = r1; i < r2+1; i++)
  {
    for (int j = c1; j < c2+1; j++)
    {
	char* str = mpz_get_str(NULL, 10, mat_element(A,i,j));
        printf("%s ", str);
    }
    printf("\n");
  }
}

// Utility function to generate matrix
void generate_matrix_util(Matrix A, int m, int tid, int numThreads)
{
    srand (time(NULL));
    int i = tid;
    mpz_t num;
    mpz_init(num);
    while(i < A->rows)
    {
        for(int j = 0; j < A->cols; j++)
        {
            mpz_set_ui(num, rand()%m);
            set_matrix_element(A, i, j, num);
        }
        i = i + numThreads;
    }
    mpz_clear(num);
}

// Generate random matrix
void generate_random_matrix(Matrix A, int m)
{
    // Use time as seed
    srand(time(0));

    // Define threadpool
    int numThreads = std::thread::hardware_concurrency();

    std::thread threads[numThreads];
    for(int i = 0; i < numThreads; i++)
    {
        threads[i] = std::thread(generate_matrix_util, A, m, i, numThreads);
    }
    for(int i = 0; i < numThreads; i++)
        threads[i].join();
}

void make_zero(Matrix A)
{
  for(int i = 0; i < A->rows; i++)
  {
    for(int j = 0; j < A->cols; j++)
      mpz_set_si(mat_element(A, i, j), 0);
  }
}