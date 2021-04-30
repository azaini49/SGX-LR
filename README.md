# INSTALL LOCAL

- Use Linux/Linux VM, see PreReqs here:
  https://github.com/intel/linux-sgx#install-the-intelr-sgx-sdk (Do not follow
the rest of those install instructions! - follow mine)
	
- Get the latest DCAP version of the SDK Installer (note: driver not needed) for your specific system from here:
  https://01.org/intel-software-guard-extensions/downloads
- Below is my execution for my ubuntu20.04 Linux VM - yours might be a little
  different
```
wget https://download.01.org/intel-sgx/sgx-linux/2.13/distro/ubuntu20.04-server/sgx_linux_x64_sdk_2.13.100.4.bin
chmod a+x sgx_linux_x64_sdk_2.13.100.4.bin
./sgx_linux_x64_sdk_2.13.100.4.bin
```
- Source environment variables according to instructions
- Get modified SGX GMP
```
mkdir sgxgmp_build
git clone https://github.com/intel/sgx-gmp.git
cd sgx-gmp
./configure --enable-cxx --enable-sgx --enable-static --disable-shared --enable-assembly --prefix=../sgxgmp_build
make
make install
```
- Clone SGX-LR
- Change SGX-LR Makefile to reflect correct SDK and GMP build directories (should be correct already)


# INSTALL NOTES FOR BIG MACHINE

- Build (enclave modified) GMP library (https://github.com/intel/sgx-gmp)
Note: make check doesn't pass - ostensibly bc of sgx defined types

In this order (Intel_SGX_Installation_Guide_Linux_2.13_Open_Source)
- Build SGX Pre-Reqs
- Build SGX Driver (without ECDSA attestation)
Note: may need to (re)execute /home/jess/sgxsdk/enviornment for correct env vars after reinitializing

- Build SGX PSW
- Build SGX SDK 
Note:
Reference: https://download.01.org/intel-sgx/sgx-linux/2.13/docs/Intel_SGX_Installation_Guide_Linux_2.13_Open_Source.pdf

- set appropriate directories in Makefile


# RUN

Source environment variables
On big machine:
```
export SGX_SDK=/home/jess/sgxsdk
export PATH=$PATH:$SGX_SDK/bin:$SGX_SDK/bin/x64
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$SGX_SDK/pkgconfig
export LD_LIBRARY_PATH=$SGX_SDK/sdk_libs
```
Locally: `source sgxsdk/environment`

Compile and run:
```
cd SGX-LR
make
./sgx_lr_exec
```

# DOCUMENTATION

**App.h**

`launch_enclave()`-> sets up secure SGX enclave, returns success/error

**Context.h**

Constructor -> instantiates N and g

**Keygen.h**

Constructor -> instantiates public and secret key vectors

`public_key()` -> Return pk

`secret_key()` -> Return sk

**Encryptor.h**

`encrypt(Matrix ciphertext, Matrix commitment, const Matrix plaintext)` -> Encrypt Matrix of plaintexts, sets ciphertext Matrix and commitment Matrix

**Evaluator.h**

`compress(Matrix compression, const Matrix ciphertext, const Matrix inp)` -> First part of decryption, sets compression Matrix

`evaluate(Matrix dest, const Matrix compression, const Matrix cmt, const mpz_t &sfk)` -> Second part of decryption, sets destination Matrix

**Logistic_Regression.h**

`train(train(Matrix xtrain_enc, Matrix xtrain_trans_enc, Matrix ytrain, Matrix cmt_xtrain, Matrix cmt_xtrain_trans, int batchSize, float learning_rate, Matrix sk_2_data)` -> Trains data and updates model weights

`predict(Matrix ypred, Matrix xtest_enc, Matrix cmt, Evaluator &eval, Public_Key pk)` -> Performs compression, applies sigmoid within enclave to create classification tag, sets ypred Matrix

`compute_performance_metrics(const Matrix ypred, const Matrix ytrue)` -> analyze output of ML model



