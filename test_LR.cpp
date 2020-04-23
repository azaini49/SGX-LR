#include "logistic_regression.h"
#include "context.h"
#include "helper.h"
#include "keygen.h"
#include "encryptor.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include "enclave_u.h"
//#include "../enclave/Enclave.h"

int main(int argc, char const *argv[])
{
    std::string dir;
    if(argc != 2)
    {
        std::cout << "Usage: ./testLR trainDir\n";
        exit(-1); 
    }
    dir = argv[1];

    // Get csv contents containing train data
    std::string trainFile = dir + "/train.csv";
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
    Matrix trainInp(xtrainRow, xtrainCol);
    Matrix testInp(xtestRow, xtestCol);
    Matrix xtrainPlain(xtrainRow, xtrainCol-2);
    Matrix xtrainPlainTrans(xtrainPlain.cols, xtrainPlain.rows);
    Matrix ytrainPlain(1, xtrainRow);
    Matrix xtestPlain(xtestRow, xtestCol-2);
    Matrix ytestPlain(1, xtestRow);
    Matrix ypred(ytestPlain.rows, ytestPlain.cols);
    Matrix ypredTrans(ypred.cols, ypred.rows);
    Matrix xtrainEnc(xtrainPlain.rows, xtrainPlain.cols);
    Matrix xtrainTransEnc(xtrainPlainTrans.rows, xtrainPlainTrans.cols);
    Matrix xtestEnc(xtestPlain.rows, xtestPlain.cols);
    Matrix cmt_xtrain(xtrainEnc.rows, 1);
    Matrix cmt_xtest(xtestEnc.rows, 1);
    Matrix cmt_xtrain_trans(xtrainTransEnc.rows, 1);

    // Populate the matices
    populate(trainInp, xtrain);
    populate(testInp, xtest);

    // Divide train data from train labels
    Matrix::splice(xtrainPlain, trainInp, 0, trainInp.rows-1, 1, trainInp.cols-2);
    xtrainPlain.transpose(xtrainPlainTrans);
    Matrix tmp1(xtrainRow, 1);
    Matrix::splice(tmp1, trainInp, 0, trainInp.rows-1, trainInp.cols-1, trainInp.cols-1);
    tmp1.transpose(ytrainPlain);

    // Divide test data from test labels
    Matrix::splice(xtestPlain, testInp, 0, testInp.rows-1, 1, testInp.cols-2);
    Matrix tmp2(xtestRow, 1);
    Matrix::splice(tmp2, testInp, 0, testInp.rows-1, testInp.cols-1, testInp.cols-1);
    tmp2.transpose(ytestPlain);

    /*******************************************************************************************************
     * Input Data Processing Ends
     * Setup For Training Begins
     *******************************************************************************************************/

    // Create entities required for 
    auto ctx = Context::Create(256);
    ecall_compute_lookup_table(ctx);
    Keygen keygen_1(ctx, xtrainPlain.cols);
    Public_Key pk_1 = keygen_1.public_key();
    std::shared_ptr<Secret_Key> app_sk_1 = std::make_unique<Secret_Key>(keygen_1.secret_key());

    Encryptor enc_1(ctx, pk_1);
    enc_1.encrypt(xtrainEnc, cmt_xtrain, xtrainPlain);
    enc_1.encrypt(xtestEnc, cmt_xtest, xtestPlain);

    Keygen keygen_2(ctx, xtrainPlainTrans.cols);
    Public_Key pk_2 = keygen_2.public_key();
    std::shared_ptr<Secret_Key> app_sk_2 = std::make_unique<Secret_Key>(keygen_2.secret_key());

    Encryptor enc_2(ctx, pk_2);
    enc_2.encrypt(xtrainTransEnc, cmt_xtrain_trans, xtrainPlainTrans);

    ecall_set_secret_key(app_sk_1, 1);
    ecall_set_secret_key(app_sk_2, 1);


    Logistic_Regression mdl(ctx, 6, 10.0);
    mdl.train(xtrainEnc, xtrainTransEnc, ytrainPlain, cmt_xtrain, cmt_xtrain_trans, 256, 0.0004);

    Evaluator eval(ctx);
    mdl.predict(ypredTrans, xtestEnc, cmt_xtest, eval);
    ypredTrans.transpose(ypred);
    mdl.compute_performance_metrics(ypred, ytestPlain);

    std::cout << "Accuracy : " << mdl.accuracy << std::endl;
    return 0;
}
