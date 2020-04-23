#include "matrix.h"
#include <thread>
#include <iostream>
#include <stdio.h>
#include <chrono>

using namespace std;

// Add two matrices
void test_matrix_addition()
{
    srand (time(NULL));
    int r1 = rand() % 10;
    int c1 = rand() % 10;
    Matrix A(r1, c1);
    Matrix::generate_random_matrix(A, 5);

    Matrix B(r1, c1);
    Matrix::generate_random_matrix(B, 5);

    Matrix res(r1, c1);
    Matrix::add(res, A, B, 5);

    mpz_t ans;
    mpz_class mod = 5;
    mpz_init(ans);
    for(int i = 0; i < r1; i++)
    {
        for(int j = 0; j < c1; j++)
        {
            mpz_add(ans, mat_element2(A, i, j), mat_element2(B, i, j));
            if(mod > 0)
                mpz_mod(ans, ans, mod.get_mpz_t());
            if(mpz_cmp(ans, mat_element2(res, i, j)) != 0)
            {
                cout << "[FAIL] Matrix addition Failed!\n";
                return;
            }
        }
    }
    cout << "[PASSED] Matrix addition Passed!\n";
}

// Multiply two matrices
void test_matrix_multiplication()
{
    srand (time(NULL));
    int r1 = rand() % 10;
    int c1 = rand() % 10;
    Matrix A(r1, c1);
    Matrix::generate_random_matrix(A, 5);

    int c2 = c1;
    int r2 = rand() % 10;
    Matrix B(r2, c2);
    Matrix::generate_random_matrix(B, 5);

    Matrix res(r1, r2);
    Matrix::multiply(res, A, B, 3);

    Matrix C(c2, r2);
    B.transpose(C);

    mpz_t x;
    mpz_init(x);
    mpz_t y;
    mpz_init(y);
    mpz_class mod = 3;
    for(int i = 0; i < r1; i++)
    {
        for(int j = 0; j < c2; j++)
        {
            mpz_set_si(y, 0);
            for(int k = 0; k < c1; k++)
            {
                mpz_mul(x, mat_element2(A, i, k), mat_element2(C, k, j));
                mpz_add(y, y, x);
            }
            mpz_mod(y, y, mod.get_mpz_t());
            if(mpz_cmp(y, mat_element2(res, i, j)) != 0)
            {
                cout << "[FAIL] Matrix multiplication Failed!\n";
                return;
            }
        }
    }
    cout << "[SUCCESS] Matrix multiplication Successful!\n";
}

// Set Matrix to identity
void test_matrix_make_identity()
{
    srand (time(NULL));
    int r1 = rand() % 10;
    int c1 = rand() % 10;
    Matrix A(r1, c1);
    Matrix::generate_random_matrix(A, 5);

    A.make_identity();
    for(int i = 0; i < r1; i++)
    {
        for(int j = 0; j < c1; j++)
        {
            if((i == j && mpz_cmp_si(mat_element2(A,i,j), 1) != 0) || (i != j && mpz_cmp_si(mat_element2(A,i,j), 0) != 0))
            {
                cout << "[FAIL] Matrix make_identity Failed!\n";
                return;
            }
        }
    }
    cout << "[PASSED] Matrix make_identity Passed!\n";
}

// Transpose of a matrix
void test_matrix_transpose()
{
    srand (time(NULL));
    int r1 = rand() % 10;
    int c1 = rand() % 10;
    Matrix A(r1, c1);
    Matrix B(c1, r1);
    Matrix::generate_random_matrix(A, 5);

    A.transpose(B);
    for(int i = 0; i < r1; i++)
    {
        for(int j = 0; j < c1; j++)
        {
            if(mpz_cmp(mat_element2(A, i, j), mat_element2(B, j, i)) != 0)
            {
                cout << "[FAIL] Matrix transpose Failed!\n";
                return;
            }
        }
    }
    cout << "[PASSED] Matrix transpose Passed!\n";
}

// Check equality
void test_matrix_equality()
{
    srand (time(NULL));
    int r1 = rand() % 10;
    int c1 = rand() % 10;
    Matrix A(r1, c1);
    Matrix::generate_random_matrix(A, 5);

    Matrix B(A);
    for(int i = 0; i < r1; i++)
    {
        for(int j = 0; j < c1; j++)
        {
            if(mpz_cmp(mat_element2(A, i, j), mat_element2(B, i, j)) != 0)
            {
                cout << "[FAIL] Matrix equality Failed!\n";
                return;
            }
        }
    }
    cout << "[PASSED] Matrix equality Passed!\n";
}

// Check row inner product
void test_row_inner_product()
{
    srand (time(NULL));
    int r1 = rand() % 6;
    int c1 = rand() % 6;
    int idx = rand() % r1;
    int idx2 = rand() % c1;
    Matrix A(r1, c1);
    Matrix::generate_random_matrix(A, 5);

    Matrix B(r1, idx2 + 1);
    Matrix::generate_random_matrix(B, 10);

    mpz_t res;
    mpz_init(res);
    Matrix::row_inner_product(res, A, B, -1, idx, idx, 0, idx2);

    mpz_t ans;
    mpz_init(ans);
    mpz_set_si(ans, 0);
    mpz_t tmp;
    mpz_init(tmp);
    for(int j = 0; j < idx2 + 1; j++)
    {
        mpz_mul(tmp, mat_element2(A, idx, j), mat_element2(B, idx, j));
        mpz_add(ans, ans, tmp);
    }
    if(mpz_cmp(ans, res) != 0)
    {
        cout << "[FAIL] Matrix Inner Product Failed!\n";
        return;
    }
    cout << "[PASSED] Matrix Inner Product Passed!\n";
}

// Check inner product
void test_inner_product()
{
    srand (time(NULL));
    int r1 = rand() % 10;
    int c1 = rand() % 10;
    int idx = rand() % r1;
    Matrix A(r1, c1);
    Matrix::generate_random_matrix(A, 5);

    Matrix B(r1, c1);
    Matrix::generate_random_matrix(B, 5);

    Matrix res(r1, 1);
    mpz_class mod = 5;
    Matrix::inner_product(res, A, B, mod);

    mpz_t ans;
    mpz_init(ans);
    mpz_t tmp;
    mpz_init(tmp);
    for(int i = 0; i < r1; i++)
    {
        mpz_set_si(ans, 0);
        for(int j = 0; j < c1; j++)
        {
            mpz_mul(tmp, mat_element2(A, idx, j), mat_element2(B, idx, j));
            mpz_add(ans, ans, tmp);
        }
        mpz_mod(ans, ans, mod.get_mpz_t());
        if(mpz_cmp(ans, mat_element2(res, i, 0)) != 0)
        {
            cout << "[FAIL] Matrix Inner Product Failed!\n";
            return;
        }
    }
    cout << "[PASSED] Matrix Inner Product Passed!\n";
}

// Test matrix splice
void test_matrix_splice()
{
    Matrix A(5, 10);
    Matrix::generate_random_matrix(A, 5);
    
    Matrix B(3, 6);
    Matrix::splice(B, A, 1, 3, 2, 7);

    printf("A:\n");
    A.print();
    printf("=========================\nB:\n");
    B.print();
     printf("=========================\n\n");

    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 6; j++)
        {
            if(mpz_cmp(mat_element2(A, i + 1, j + 2), mat_element2(B, i, j)) != 0)
            {
                gmp_printf("A(%d, %d) : %Zd\n", i+1, j+2, mat_element2(A, i + 1, j + 2));
                gmp_printf("B(%d, %d) : %Zd\n", i, j, mat_element2(B, i, j));
                cout << "[FAIL] Matrix Splice Failed!\n";
                return;
            }
        }
    }
    cout << "[PASSED] Matrix Splice Passed!\n";
}

// Test matrix addition performance test
void test_perf_addition()
{
    Matrix A(1000, 1000);
    Matrix::generate_random_matrix(A, 500);

    Matrix B(1000, 1000);
    Matrix::generate_random_matrix(B, 500);

    Matrix C(1000, 1000);

    auto t1 = std::chrono::high_resolution_clock::now();
    Matrix::add(C, B, A);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Matrix Addition (1000 x 1000) : " << duration << " microseconds" << endl;
}

// Test matrix multiplication performance test
void test_perf_multiplication()
{
    Matrix A(1000, 1000);
    Matrix::generate_random_matrix(A, 500);

    Matrix B(1000, 1000);
    Matrix::generate_random_matrix(B, 500);

    Matrix C(1000, 1000);

    auto t1 = std::chrono::high_resolution_clock::now();
    Matrix::multiply(C, B, A);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Matrix Multiplication (1000 x 1000) : " << duration << " microseconds" << endl;
}

// Test matrix transpose performance test
void test_perf_transpose()
{
    Matrix A(1000, 1000);
    Matrix::generate_random_matrix(A, 500);

    Matrix C(1000, 1000);

    auto t1 = std::chrono::high_resolution_clock::now();
    A.transpose(C);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Matrix Transpose (1000 x 1000) : " << duration << " microseconds" << endl;
}

// Test matrix inner product performance test
void test_perf_inner_product()
{
    Matrix A(1000, 1000);
    Matrix::generate_random_matrix(A, 500);

    Matrix B(1000, 1000);
    Matrix::generate_random_matrix(B, 500);

    Matrix C(1000, 1);

    auto t1 = std::chrono::high_resolution_clock::now();
    Matrix::inner_product(C, B, A);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Matrix Inner Product (1000 x 1000) : " << duration << " microseconds" << endl;
}

int main(int argc, char const *argv[])
{
    while(true)
    {
        printf("+------------------------------------------------------------------------+\n");
        printf("| Tests                                                                  |\n" );
        printf("+------------------------------------------------------------------------+\n");
        printf("| 1. Test Matrix Addition                                                |\n");
        printf("| 2. Test Matrix Multiplication                                          |\n");
        printf("| 3. Test Matrix Identity                                                |\n");
        printf("| 4. Test Matrix Transpose                                               |\n");
        printf("| 5. Test Matrix Row Inner Product                                       |\n");
        printf("| 6. Test Matrix Inner Product                                           |\n");
        printf("| 7. Test Matrix Splice                                                  |\n");
        printf("| 8. Test Matrix Addition performance                                    |\n");
        printf("| 9. Test Matrix Multiplication performance                              |\n");
        printf("| 10. Test Matrix Transpose performance                                  |\n");
        printf("| 11. Test Matrix Inner Product performance                              |\n");
        printf("+------------------------------------------------------------------------+\n");
        int selection = 0;
        bool invalid = true;
        do
        {
            printf("> Run example (1 ~ 11) or exit (0): ");
            scanf("%d", &selection);
            if (selection < 0 || selection > 11)
            {
                invalid = false;
            }
            else
            {
                invalid = true;
            }
            if (!invalid)
            {
                printf("  Invalid option: type 0 ~ 3\n");
                
            }
        } while (!invalid);
        printf("\n");
        switch (selection)
        {
            case 1:
                test_matrix_addition();
                break;
            case 2:
                test_matrix_multiplication();
                break;
            case 3:
                test_matrix_make_identity();
                break;
            case 4:
                test_matrix_transpose();
                break;
            case 5:
                test_row_inner_product();
                break;
            case 6:
                test_inner_product();
                break;
            case 7:
                test_matrix_splice();
                break;
            case 8:
                test_perf_addition();
                break;
            case 9:
                test_perf_multiplication();
                break;
            case 10:
                test_perf_transpose();
                break;
            case 11:
                test_perf_inner_product();
                break;
            case 0:
                return 0;
        }
    }
    return 0;
}
