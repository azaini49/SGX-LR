#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_satus_t etc. */


#include <stdlib.h> /* for size_t */

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

#ifndef OCALL_PRINT_STRING_DEFINED__
#define OCALL_PRINT_STRING_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_string, (const char* str));
#endif
#ifndef OCALL_PRINT_MATRIX_DEFINED__
#define OCALL_PRINT_MATRIX_DEFINED__
void SGX_UBRIDGE(SGX_NOCONVENTION, ocall_print_matrix, (uint8_t* val, int len));
#endif

sgx_status_t enclave_service(sgx_enclave_id_t eid, int* retval, void* task_queue);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
