#ifndef __TEE_SYSCALL_H
#define __TEE_SYSCALL_H

#include <tee_client_api.h>

struct tee_ioctl_param;

TEEC_Result tee_syscall_process(size_t num_params,
                                struct tee_ioctl_param *params);

#endif //__TEE_SYSCALL_H
