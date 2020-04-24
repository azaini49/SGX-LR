#ifndef ENCLAVE_T_H__
#define ENCLAVE_T_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include "sgx_edger8r.h" /* for sgx_ocall etc. */

#include "../tools/sgx_tgmp.h"
#include "../tools/matrix_shared.h"
#include "../tools/secret_key.h"
#include "../tools/gmpxx.h"
#include "../include/Queue.h"
#include "../include/sync_utils.hpp"

#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

int enclave_service(void* task_queue);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
