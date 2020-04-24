#include "../include/App.h"
#include "sgx_urts.h"
#include "enclave_u.h"


/* Global EID shared by multiple threads */
extern sgx_enclave_id_t global_eid;
Queue* task_queue;

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

int main(int argc, char const *argv[])
{

    int enclave_status = launch_enclave();
    if(enclave_status != SGX_SUCCESS)
    {
        std::cout << "Failed to launch enclave. Exiting...\n";
        exit(EXIT_FAILURE); 
    }

    /**
     * TODO : Call function to initialize the enclave pk/sk pair
     */


    // Retrive training and testing data to process from csv files
    std::string dir;
    if(argc != 2)
    {
        std::cout << "Usage: ./app_lr trainDir\n";
        exit(-1); 
    }
    dir = argv[1];

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
    Matrix xtrainEnc = mat_init(xtrainPlain->rows, xtrainPlain->cols);
    Matrix xtrainTransEnc = mat_init(xtrainPlainTrans->rows, xtrainPlainTrans->cols);
    Matrix xtestEnc = mat_init(xtestPlain->rows, xtestPlain->cols);
    Matrix cmt_xtrain = mat_init(xtrainEnc->rows, 1);
    Matrix cmt_xtest = mat_init(xtestEnc->rows, 1);
    Matrix cmt_xtrain_trans = mat_init(xtrainTransEnc->rows, 1);

    // Populate the matices
    populate(trainInp, xtrain);
    populate(testInp, xtest);

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

    /*******************************************************************************************************
     * Input Data Processing Ends
     * Setup For Training Begins
     *******************************************************************************************************/

    // Create entities required for 
    auto ctx = Context::Create(256);

    // Make a request to setup the lookup table for discrete log
    Request req = init_request(GENERATE_LOOKUP_TABLE, 1);
    mpz_get_str(req->p, BASE, ctx->p);
    mpz_get_str(req->g, BASE, ctx->g);
    req->limit = 10;
    make_request(req);

    // Generate pk and sk to encrypt xtrain and xtest
    Keygen keygen_1(ctx, xtrainPlain->cols);
    Public_Key pk_1 = keygen_1.public_key();
    std::shared_ptr<Secret_Key> app_sk_1 = std::make_unique<Secret_Key>(keygen_1.secret_key());

    // Instantiate encryptor
    Encryptor enc_1(ctx, pk_1);
    enc_1.encrypt(xtrainEnc, cmt_xtrain, xtrainPlain);
    enc_1.encrypt(xtestEnc, cmt_xtest, xtestPlain);

    // Generate pk and sk to encrypt xtrainTrans
    Keygen keygen_2(ctx, xtrainPlainTrans->cols);
    Public_Key pk_2 = keygen_2.public_key();
    std::shared_ptr<Secret_Key> app_sk_2 = std::make_unique<Secret_Key>(keygen_2.secret_key());

    // Instantiate encryptor
    Encryptor enc_2(ctx, pk_2);
    enc_2.encrypt(xtrainTransEnc, cmt_xtrain_trans, xtrainPlainTrans);

    // Generate request for the enclave to setup sk_1 and sk_2 
    req->key_id = 1;
    req->input_1 = app_sk_1->data();
    make_request(req);

    req->key_id = 2;
    req->input_1 = app_sk_2->data();
    make_request(req);

    //TODO : Encrypt cmmmitments using the enclave pk

    // Instantiate LR model to train
    Logistic_Regression mdl(ctx, 6, 10.0);
    mdl.train(xtrainEnc, xtrainTransEnc, ytrainPlain, cmt_xtrain, cmt_xtrain_trans, 256, 0.0004);

    Evaluator eval(ctx);
    mdl.predict(ypredTrans, xtestEnc, cmt_xtest, eval);
    transpose(ypred, ypredTrans);
    mdl.compute_performance_metrics(ypred, ytestPlain);

    std::cout << "Accuracy : " << mdl.accuracy << std::endl;

    
    return 0;
}
