#include <stdarg.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <tee_supplicant.h>

#ifndef __aligned
#define __aligned(x) __attribute__((__aligned__(x)))
#endif
#include <linux/tee.h>

#include "tee_syscall.h"

static inline bool check_params_impl(struct tee_ioctl_param *params, size_t given_num_params, size_t num_params, ...) {
    if (given_num_params != num_params) {
        return false;
    }
    va_list args;
    va_start(args, num_params);
    for (size_t i = 0; i < num_params; i++) {
        if ((params[i].attr & TEE_IOCTL_PARAM_ATTR_TYPE_MASK) != va_arg(args, uint64_t)) {
            return false;
        }
    }
    va_end(args);
    return true;
}

#define check_params(params, num_params, ...) \
    check_params_impl(params, num_params, sizeof((int[]){__VA_ARGS__}) / sizeof(int), ##__VA_ARGS__)

static int handle_openat(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int dirfd = params[0].b;
    char *pathname = tee_supp_param_to_va(params + 1);
    int flags = params[0].c;
    int ret = syscall(SYS_openat, dirfd, pathname, flags);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_close(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int fd = params[0].b;
    int ret = syscall(SYS_close, fd);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_read(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int fd = params[0].b;
    void *read_buf = tee_supp_param_to_va(params + 1);
    size_t count = MEMREF_SIZE(params + 1);
    int ret = syscall(SYS_read, fd, read_buf, count);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_write(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int fd = params[0].b;
    void *write_buf = tee_supp_param_to_va(params + 1);
    size_t count = MEMREF_SIZE(params + 1);
    int ret = syscall(SYS_write, fd, write_buf, count);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_pread(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int fd = params[0].b;
    off_t ofs = params[0].c;
    void *read_buf = tee_supp_param_to_va(params + 1);
    size_t count = MEMREF_SIZE(params + 1);
    int ret = syscall(SYS_read, fd, read_buf, count,ofs);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_pwrite(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int fd = params[0].b;
    off_t ofs = params[0].c;
    void *write_buf = tee_supp_param_to_va(params + 1);
    size_t count = MEMREF_SIZE(params + 1);
    int ret = syscall(SYS_pwrite64, fd, write_buf, count,ofs);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_access(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_MEMREF_INPUT,
                      TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
    )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int amode = params[0].b;
    char *pathname = tee_supp_param_to_va(params + 1);
    int ret = syscall(SYS_faccessat, amode, pathname);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

static int handle_lseek(size_t num_params, struct tee_ioctl_param *params) {
    if (!check_params(params, num_params,
                          TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                          TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_INPUT,
                          TEE_IOCTL_PARAM_ATTR_TYPE_VALUE_OUTPUT
            )) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }

    int fd = params[0].b;
    long offset = params[0].c;
    int whence = params[1].a;
    int ret = syscall(SYS_lseek, fd, offset,whence);
    params[2].a = ret;
    return TEEC_SUCCESS;
}

TEEC_Result tee_syscall_process(size_t num_params,
                                struct tee_ioctl_param *params) {
    if (num_params == 0) {
        return TEEC_ERROR_BAD_PARAMETERS;
    }
    int syscall_id = params[0].a;
    switch (syscall_id) {
        case SYS_openat:
            return handle_openat(num_params, params);
        case SYS_close:
            return handle_close(num_params, params);
        case SYS_read:
            return handle_read(num_params, params);
        case SYS_write:
            return handle_write(num_params, params);
        case SYS_pread64:
            return handle_pread(num_params, params);
        case SYS_pwrite64:
            return handle_pwrite(num_params, params);
        case SYS_faccessat:
            return handle_access(num_params, params);
        case SYS_lseek:
            return handle_lseek(num_params, params);
        default:
            return TEEC_ERROR_BAD_PARAMETERS;
    }
}
