#include "keygen.h"
#include "encryptor.h"
#include "evaluator.h"
#include "helper.h"
#include <math.h>
#include <chrono>

using namespace std;
static std::map<mpz_class, mpz_class> lookup;

int compute_lookup_table(std::shared_ptr<Context> ctx)
{
    mpz_t tmp;
    mpz_init(tmp);

    mpz_class i = 0;
    mpz_class limit = pow(2, 15);
    while(i < limit)
    {
        mpz_powm(tmp, ctx->g, i.get_mpz_t(), ctx->p);
        //std::unique_lock<std::mutex> locker(gaurd);
        lookup[mpz_class{tmp}] = i;
        mpz_invert(tmp, tmp, ctx->p);
        lookup[mpz_class{tmp}] = -i;
        //locker.unlock();
        i = i + 1;
    }
    mpz_clear(tmp);
    return COMPLETED;
}

void test_context_creation()
{
    auto t1 = std::chrono::high_resolution_clock::now();
    auto ctx = Context::Create(1024);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Context Creation : " << duration << " microseconds" << endl;
}

void test_key_generation()
{
    auto ctx = Context::Create(1024);

    auto t1 = std::chrono::high_resolution_clock::now();
    Keygen kgen(ctx, 2048);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Key Generation : " << duration << " microseconds" << endl;
}

void test_compression_evaluation()
{
    // Initialize input matrices
    int klen = 2048;
    Matrix A = mat_init(1000, klen);
    Matrix B = mat_init(1, klen);
    generate_random_matrix(A, 10);
    generate_random_matrix(B, 10);

    // Initialize context
    auto ctx = Context::Create(256);

    // Generate keys
    Keygen kgen(ctx, klen);
    Public_Key pk = kgen.public_key();
    Secret_Key sk = kgen.secret_key();

    compute_lookup_table(ctx);

    // Store secret function key
    mpz_t sfk;
    mpz_init(sfk);
    row_inner_product(sfk, sk.data(), B, 0);

    // Initialize encryptor and evaluator
    Encryptor enc(ctx, pk);
    Evaluator eval(ctx);
    

    // Store encrypted input in cipher and cmt
    Matrix cipher = mat_init(A->rows, A->cols);
    Matrix cmt = mat_init(A->rows, 1);

    // Store compression result in compress
    Matrix compression = mat_init(cipher->rows, 1);

    // Encrypt input
    auto t1 = std::chrono::high_resolution_clock::now();
    enc.encrypt(cipher, cmt, A);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Encryption : " << duration << " microseconds" << endl;


    // Compress ciphertext
    t1 = std::chrono::high_resolution_clock::now();
    eval.compress(compression, cipher, B);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Compression : " << duration << " microseconds" << endl;


    // Evluate inner product
    Matrix ip = mat_init(cipher->rows, 1);
    t1 = std::chrono::high_resolution_clock::now();
    //eval.evaluate(ip, compression, cmt, sfk);
    t2 = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    cout << "Evaluation : " << duration << " microseconds" << endl;

    mpz_t tmp;
    mpz_init(tmp);
    for(int i = 0; i < A->rows; i++)
    {
        row_inner_product(tmp, A, B, i, 0);
        if(mpz_cmp(tmp, mat_element(ip, i, 0)) != 0)
        {
            gmp_printf("Row : %d    Expected : %Zd  Computed : %Zd\n", i, tmp, mat_element(ip, i, 0));
            cout << "[FAILED] Compression-Evaluation Incorrect\n";
            mpz_clear(tmp);
            mpz_clear(sfk);
            return;
        }
    }
    mpz_clear(tmp);
    mpz_clear(sfk);
    cout << "[SUCCESS] Compression-Evaluation Successful\n";
}


int main(int argc, char const *argv[])
{
    while(true)
    {
        printf("+---------------------------------------------------------------------+\n");
        printf("| Tests                                                               |\n" );
        printf("+---------------------------------------------------------------------+\n");
        printf("| 1. Test Context Creation Performance                                |\n");
        printf("| 2. Test Key Generation Performance                                  |\n");
        printf("| 3. Test Compression-Evaluation                                      |\n");
        printf("+---------------------------------------------------------------------+\n");
        int selection = 0;
        bool invalid = true;
        do
        {
            printf("> Run example (1 ~ 3) or exit (0): ");
            scanf("%d", &selection);
            if (selection < 0 || selection > 3)
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
                test_context_creation();
                break;
            case 2:
                test_key_generation();
                break;
            case 3:
                test_compression_evaluation();
                break;
            case 0:
                return 0;
        }
    }
    return 0;
}
