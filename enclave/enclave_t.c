#include "enclave_t.h"

#include "sgx_trts.h" /* for sgx_ocalloc, sgx_is_outside_enclave */

#include <errno.h>
#include <string.h> /* for memcpy etc */
#include <stdlib.h> /* for malloc/free etc */

#define CHECK_REF_POINTER(ptr, siz) do {	\
	if (!(ptr) || ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)

#define CHECK_UNIQUE_POINTER(ptr, siz) do {	\
	if ((ptr) && ! sgx_is_outside_enclave((ptr), (siz)))	\
		return SGX_ERROR_INVALID_PARAMETER;\
} while (0)


typedef struct ms_enclave_service_t {
	int ms_retval;
	void* ms_task_queue;
} ms_enclave_service_t;

typedef struct ms_ocall_print_string_t {
	char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_print_matrix_t {
	uint8_t* ms_val;
	int ms_len;
} ms_ocall_print_matrix_t;

static sgx_status_t SGX_CDECL sgx_enclave_service(void* pms)
{
	CHECK_REF_POINTER(pms, sizeof(ms_enclave_service_t));
	ms_enclave_service_t* ms = SGX_CAST(ms_enclave_service_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	void* _tmp_task_queue = ms->ms_task_queue;


	ms->ms_retval = enclave_service(_tmp_task_queue);


	return status;
}

SGX_EXTERNC const struct {
	size_t nr_ecall;
	struct {void* ecall_addr; uint8_t is_priv;} ecall_table[1];
} g_ecall_table = {
	1,
	{
		{(void*)(uintptr_t)sgx_enclave_service, 0},
	}
};

SGX_EXTERNC const struct {
	size_t nr_ocall;
	uint8_t entry_table[2][1];
} g_dyn_entry_table = {
	2,
	{
		{0, },
		{0, },
	}
};


sgx_status_t SGX_CDECL ocall_print_string(const char* str)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_str = str ? strlen(str) + 1 : 0;

	ms_ocall_print_string_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_string_t);
	void *__tmp = NULL;

	ocalloc_size += (str != NULL && sgx_is_within_enclave(str, _len_str)) ? _len_str : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_string_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_string_t));

	if (str != NULL && sgx_is_within_enclave(str, _len_str)) {
		ms->ms_str = (char*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_str);
		memcpy((void*)ms->ms_str, str, _len_str);
	} else if (str == NULL) {
		ms->ms_str = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	status = sgx_ocall(0, ms);


	sgx_ocfree();
	return status;
}

sgx_status_t SGX_CDECL ocall_print_matrix(uint8_t* val, int len)
{
	sgx_status_t status = SGX_SUCCESS;
	size_t _len_val = len * sizeof(*val);

	ms_ocall_print_matrix_t* ms = NULL;
	size_t ocalloc_size = sizeof(ms_ocall_print_matrix_t);
	void *__tmp = NULL;

	ocalloc_size += (val != NULL && sgx_is_within_enclave(val, _len_val)) ? _len_val : 0;

	__tmp = sgx_ocalloc(ocalloc_size);
	if (__tmp == NULL) {
		sgx_ocfree();
		return SGX_ERROR_UNEXPECTED;
	}
	ms = (ms_ocall_print_matrix_t*)__tmp;
	__tmp = (void *)((size_t)__tmp + sizeof(ms_ocall_print_matrix_t));

	if (val != NULL && sgx_is_within_enclave(val, _len_val)) {
		ms->ms_val = (uint8_t*)__tmp;
		__tmp = (void *)((size_t)__tmp + _len_val);
		memcpy(ms->ms_val, val, _len_val);
	} else if (val == NULL) {
		ms->ms_val = NULL;
	} else {
		sgx_ocfree();
		return SGX_ERROR_INVALID_PARAMETER;
	}
	
	ms->ms_len = len;
	status = sgx_ocall(1, ms);


	sgx_ocfree();
	return status;
}

