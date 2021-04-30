#include "App.h"
#include <inttypes.h>
#include <string>
#include <chrono>

/* Global EID shared by multiple threads */
extern sgx_enclave_id_t global_eid;
Queue* task_queue;

extern int SECURITY_BITS;

// Variable indidcating whether the enclave thread has been launched
bool enclave_running = false;

// Make an ecall to the enclave_service
void enclave_thread()
{
    int resp = 0;
    enclave_service(global_eid, &resp, task_queue);
}

int launch_enclave()
{
    // Initialize the enclave
    int enclave_status = initialize_enclave();

    // Instantiate the request queue
    task_queue = new Queue();

    // Set enclave thread status to true
    enclave_running = true;

    // Start the enclave thread
    std::thread e_thread = std::thread(&enclave_thread);
    e_thread.detach();
    return enclave_status;
}

/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate
     * the input string to prevent buffer overflow.
     */
    printf("%s", str);
}

void ocall_print_matrix(uint8_t* val, int len)
{
    std::cout << "User side ocall\n";
    Matrix m = deserialize_matrix(val);
    print_matrix(m);
}

int main(int argc, char const *argv[])
{

    std::string dir;
    if(argc < 2)
        dir = "./Lingspam";
    else
        dir = argv[1];

    std::cout << "\n===== LAUNCHING ENCLAVE ====\n";
    // Launch enclave
    int enclave_status = launch_enclave();
    if(enclave_status != SGX_SUCCESS)
    {
        std::cout << "Failed to launch enclave. Exiting...\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "=> Enclave Created\n";

// JESS's new sanity test for crypto

    // Retrive training and testing data to process from csv files
    // Get csv contents containing train data
    std::string trainFile(dir + "/train.csv");
    std::vector<std::vector<int> > xtrain;
    readFile(trainFile, xtrain);
    std::cout << "Read train data.\n";

    // Get csv contents containing test data
    std::string testFile = dir + "/test.csv";
    std::vector<std::vector<int> > xtest;
    readFile(testFile, xtest);
    std::cout << "Read test data.\n" ;

    // Get content dimensions
    int xtestRow = xtest.size();
    int xtestCol = xtest[0].size();
    int xtrainRow = xtrain.size();
    int xtrainCol = xtrain[0].size();
    std::cout << "Xtrain row: " << xtrainRow << std::endl;
    std::cout << "Xtrain col: " << xtrainCol-2 << std::endl;
    std::cout << "Xtest row: " << xtestRow << std::endl;
    std::cout << "Xtest col: " << xtestCol-2 << std::endl;

    // Instantiate matrices
    Matrix trainInp = mat_init(xtrainRow, xtrainCol);
    Matrix testInp = mat_init(xtestRow, xtestCol);
    Matrix xtrainPlain = mat_init(xtrainRow, xtrainCol-2);
    Matrix xtrainPlainTrans = mat_init(xtrainPlain->cols, xtrainPlain->rows);
    Matrix ytrainPlain = mat_init(1, xtrainRow);
    Matrix xtestPlain = mat_init(xtestRow, xtestCol-2);
    Matrix ytestPlain = mat_init(1, xtestRow);
    Matrix ypred = mat_init(ytestPlain->rows, ytestPlain->cols);
    Matrix ypredTrans = mat_init(ypred->cols, ypred->rows);
    Matrix ypredTrans2 = mat_init(ypred->cols, ypred->rows);
    Matrix xtrainEnc = mat_init(xtrainPlain->rows, xtrainPlain->cols);
    Matrix xtrainTransEnc = mat_init(xtrainPlainTrans->rows, xtrainPlainTrans->cols);
    Matrix xtestEnc = mat_init(xtestPlain->rows, xtestPlain->cols);
    Matrix cmt_xtrain = mat_init(xtrainEnc->rows, 1);
    Matrix cmt_xtest = mat_init(xtestEnc->rows, 1);
    Matrix cmt_xtrain_trans = mat_init(xtrainTransEnc->rows, 1);

    // Populate the matices
    populate(trainInp, xtrain);
    populate(testInp, xtest);

    // Populate the matices
    // populate(testInp, xtest);
    std::cout << "=> Populated test matrix" << std::endl;

    // Divide train data from train labels
    mat_splice(xtrainPlain, trainInp, 0, trainInp->rows-1, 1, trainInp->cols-2);
    transpose(xtrainPlainTrans, xtrainPlain);
    Matrix tmp1 = mat_init(xtrainRow, 1);
    mat_splice(tmp1, trainInp, 0, trainInp->rows-1, trainInp->cols-1, trainInp->cols-1);
    transpose(ytrainPlain, tmp1);

    // Divide test data from test labels
    mat_splice(xtestPlain, testInp, 0, testInp->rows-1, 1, testInp->cols-2);
    Matrix tmp2 = mat_init(xtestRow, 1);
    mat_splice(tmp2, testInp, 0, testInp->rows-1, testInp->cols-1, testInp->cols-1);
    transpose(ytestPlain, tmp2);
/*
    for (int i = 0; i < 5; i++){
      printf("x plain: %ld\n", mpz_get_si(mat_element(xtestPlain, 0, i)));
      //std::cout << "output : " << mat_element(ypred, 0, i) << std::endl;
    }
*/

    std::chrono::time_point<std::chrono::high_resolution_clock>  start, end;
    start = std::chrono::high_resolution_clock::now();

    // Create entities required for
    SECURITY_BITS = 1024;

//start = std::chrono::high_resolution_clock::now();
auto ctx = Context::Create(SECURITY_BITS, 500, 500);

//end = std::chrono::high_resolution_clock::now();
//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
//std::cout << "=> CONTEXT : " << duration << " microseconds\n";
    



//    auto ctx = Context::Create(SECURITY_BITS, 500, 500);

    Matrix dummy = NULL;

    // Generate pk and sk to encrypt xtest
//start = std::chrono::high_resolution_clock::now();
Keygen keygen_1(ctx, xtestPlain->cols, false);
//end = std::chrono::high_resolution_clock::now();
//duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
//std::cout << "=> KEY GEN : " << duration << " microseconds\n";


   // Keygen keygen_1(ctx, xtestPlain->cols, true);
    Public_Key pk_1 = keygen_1.public_key();

    std::shared_ptr<Secret_Key> app_sk_1 = std::make_unique<Secret_Key>(keygen_1.secret_key());

    //Matrix xtestEnc = mat_init(xtestPlain->rows, xtestPlain->cols);
    //Matrix cmt_xtest = mat_init(xtestEnc->rows, 1);
    Matrix cca_ct_xtest = mat_init(xtestEnc->rows, 1);

    // Instantiate encryptor
    Encryptor enc_1(ctx, pk_1);
//start = std::chrono::high_resolution_clock::now();
enc_1.encrypt(xtestEnc, cmt_xtest, xtestPlain, cca_ct_xtest);

//end = std::chrono::high_resolution_clock::now();
//duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
//std::cout << "=> Encrypt : " << duration << " microseconds\n";
//    enc_1.encrypt(xtestEnc, cmt_xtest, xtestPlain, cca_ct_xtest);

    enc_1.encrypt(xtrainEnc, cmt_xtrain, xtrainPlain);

     // Generate pk and sk to encrypt xtrainTrans
    Keygen keygen_2(ctx, xtrainPlainTrans->cols, false);
    Public_Key pk_2 = keygen_2.public_key();
    std::shared_ptr<Secret_Key> app_sk_2 = std::make_unique<Secret_Key>(keygen_2.secret_key());

    // Instantiate encryptor
    Encryptor enc_2(ctx, pk_2);
    enc_2.encrypt(xtrainTransEnc, cmt_xtrain_trans, xtrainPlainTrans);

    Request req = serialize_request(SET_FE_SECRET_KEY, app_sk_1->data(), dummy, dummy, dummy, mpz_class{ctx->N}, mpz_class{ctx->Ns}, mpz_class{ctx->g});
    req->key_id = 1;
    make_request(req);

    req = serialize_request(SET_FE_SECRET_KEY, app_sk_2->data(), dummy, dummy, dummy, mpz_class{ctx->N}, mpz_class{ctx->Ns}, mpz_class{ctx->g});
    req->key_id = 2;
    make_request(req);

    end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "=> Setup : " << duration << " microseconds\n";
start = std::chrono::high_resolution_clock::now();
    
// Instantiate LR model to train
    Logistic_Regression mdl(ctx, 6, 10.0);
    mdl.train(xtrainEnc, xtrainTransEnc, ytrainPlain, cmt_xtrain, cmt_xtrain_trans, 256, 0.0004, app_sk_2->data());


    Evaluator eval(ctx);

    mpz_t sky;
    mpz_init(sky);

    mpz_t sky_cca_j1;
    mpz_t sky_cca_j2;

    mpz_init(sky_cca_j1);
    mpz_init(sky_cca_j2);

// start = std::chrono::high_resolution_clock::now();
   // keygen_1.key_der(mdl.weights, sky, sky_cca_j1, sky_cca_j2);
//end = std::chrono::high_resolution_clock::now();
//duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
//std::cout << "=> Ker Der : " << duration << " microseconds\n";
    
/*
 start = std::chrono::high_resolution_clock::now();
eval.cca_check(cca_ct_xtest, cmt_xtest, mdl.weights, sky_cca_j1, sky_cca_j2);
end = std::chrono::high_resolution_clock::now();
duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
std::cout << "=> CCA Check : " << duration << " microseconds\n";

*/


//    req = serialize_request(SET_SFK, weights, dummy, dummy, dummy, 0, 0, 0, 0);
//    make_request(req);


    mdl.predict(ypredTrans, xtestEnc, cmt_xtest, eval, pk_1);
    
//    Timer timer("Prediction Time");

 
//    Logistic_Regression mdl(ctx, 6, 10.0);
//    mdl.train(xtrainEnc, xtrainTransEnc, ytrainPlain, cmt_xtrain, cmt_xtrain_trans, 256, 0.0004);

    Matrix compression = mat_init(xtestEnc->rows, 1);
   
/*
    start = std::chrono::high_resolution_clock::now();
eval.compress(compression, xtestEnc, mdl.weights);

end = std::chrono::high_resolution_clock::now();
duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
std::cout << "=> compress : " << duration << " microseconds\n";

 start = std::chrono::high_resolution_clock::now();
 eval.evaluate(ypredTrans2, compression, cmt_xtest, sky);

end = std::chrono::high_resolution_clock::now();
duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
std::cout << "=> eval : " << duration << " microseconds\n";
*/

  
     transpose(ypred, ypredTrans2);
    //mdl.compute_performance_metrics(ypred, ytestPlain);


    /*    for (int i = 0; i < compression->rows; i++){
      printf("y: %ld\n", mpz_get_si(mat_element(compression, i, 0)));
    }

    req = serialize_request(FINAL_PREDICTION, pk_1.data(), ypredTrans, compression, cmt_xtest, mpz_class{ctx->N}, mpz_class{ctx->Ns}, mpz_class{ctx->g});
    make_request(req);
   
*/  
//    transpose(ypred, ypredTrans);
/*    for (int i = 0; i < ypred->cols; i++){
      printf("y: %ld\n", mpz_get_si(mat_element(ypred, 0, i)));
    }
*/  
    
    mdl.compute_performance_metrics(ypred, ytestPlain);
    std::cout << " " << std::endl;
    std::cout << "======== PERFORMANCE =======" << std::endl;
    std::cout << "Accuracy : " << mdl.accuracy << std::endl;
//    std::cout << "Precision : " << mdl.precision << std::endl;
//    std::cout << "Recall : " << mdl.recall << std::endl;
//    std::cout << "F1 : " << mdl.f1 << std::endl;
//    std::cout << "tn : " << mdl.tn << " tp : " << mdl.tp << " fn : " << mdl.fn << " fp : " << mdl.fp << std::endl;

    std::cout << " " << std::endl;



    std::cout << "\n====== EXITING ENCLAVE =====\n";
    req = serialize_request(EXIT_ENCLAVE, dummy, dummy, dummy, dummy);
    make_request(req);
    sgx_destroy_enclave(global_eid);
    std::cout << "=> Enclave Destroyed\n";


    return 0;
}
