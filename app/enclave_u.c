#include "enclave_u.h"
#include <errno.h>

typedef struct ms_enclave_service_t {
	int ms_retval;
	void* ms_task_queue;
} ms_enclave_service_t;

static const struct {
	size_t nr_ocall;
	void * table[1];
} ocall_table_enclave = {
	0,
	{ NULL },
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

