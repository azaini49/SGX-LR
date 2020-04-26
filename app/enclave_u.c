#include "enclave_u.h"
#include <errno.h>

typedef struct ms_enclave_service_t {
	int ms_retval;
	void* ms_task_queue;
} ms_enclave_service_t;

typedef struct ms_ocall_print_string_t {
	const char* ms_str;
} ms_ocall_print_string_t;

typedef struct ms_ocall_print_matrix_t {
	const char* ms_mat;
} ms_ocall_print_matrix_t;

static sgx_status_t SGX_CDECL enclave_ocall_print_string(void* pms)
{
	ms_ocall_print_string_t* ms = SGX_CAST(ms_ocall_print_string_t*, pms);
	ocall_print_string(ms->ms_str);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_ocall_print_matrix(void* pms)
{
	ms_ocall_print_matrix_t* ms = SGX_CAST(ms_ocall_print_matrix_t*, pms);
	ocall_print_matrix(ms->ms_mat);

	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * table[2];
} ocall_table_enclave = {
	2,
	{
		(void*)enclave_ocall_print_string,
		(void*)enclave_ocall_print_matrix,
	}
};
sgx_status_t enclave_service(sgx_enclave_id_t eid, int* retval, void* task_queue)
{
	sgx_status_t status;
	ms_enclave_service_t ms;
	ms.ms_task_queue = task_queue;
	status = sgx_ecall(eid, 0, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

