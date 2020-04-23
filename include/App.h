#pragma once

#include <sgx_eid.h>
#define HAVE_SGX 1

extern sgx_enclave_id_t global_eid;

#include "enclave_init.h"
#include <iostream>
#include "../enclave/Queue.h"
#include "helper.h"
#include <thread>
#include "../app/logistic_regression.h"
#include "../app/context.h"
#include "../app/helper.h"
#include "../app/keygen.h"
#include "../app/encryptor.h"