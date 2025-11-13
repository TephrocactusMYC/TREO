#ifndef __PTA_ELF_TA_LOADER_H
#define __PTA_ELF_TA_LOADER_H

#define PTA_ELF_TA_LOADER_UUID \
		{ 0x20492a93, 0x3941, 0x4ed0, \
			{ 0x9c, 0x0d, 0x05, 0x8e, 0x55, 0x98, 0x9f, 0x50 } }

#define PTA_ELF_TA_LOADER_CMD_SYSCALL		0

// int sys_openat(int dirfd, const char *pathname, int flags)
typedef struct {
	int dirfd;
	const char *pathname;
	size_t pathname_len;
	int flags;
} sys_openat_args_t;

// int sys_close(int fd)
typedef struct {
	int fd;
} sys_close_args_t;

// ssize_t read(int fd, void *buf, size_t count)
typedef struct {
	int fd;
	void *buf;
	size_t count;
} sys_read_args_t;

// ssize_t write(int fd, const void *buf, size_t count)
typedef struct {
	int fd;
	void *buf;
	size_t count;
} sys_write_args_t;

// ssize_t pread(int fd, void *buf, size_t size, off_t ofs)
typedef struct {
	int fd;
	void *buf;
	size_t count;
	long ofs;
} sys_pread_args_t;

// ssize_t pwrite(int fd, void *buf, size_t size, off_t ofs)
typedef struct {
	int fd;
	void *buf;
	size_t count;
	long ofs;
} sys_pwrite_args_t;

// int access(const char *filename, int amode)
typedef struct {
	const char *filename;
	size_t pathname_len;
	int amode;
} sys_access_args_t;

// int arch_prctl(int code, unsigned long addr)
//typedef struct {
//	int code;
//	unsigned long addr;
//} sys_arch_prctl_args_t;

// long __lseek(int fd, off_t offset, int whence)
typedef struct {
	int fd;
	long offset;
	int whence;
} sys_lseek_args_t;

// ssize_t readv(int fd, const struct iovec *iov, int count)
struct iovec {
	void *iov_base;
	size_t iov_len;
};

typedef struct {
	int fd;
	const struct iovec *iov;
	int count;
} sys_readv_args_t;

#endif //__PTA_ELF_TA_LOADER_H
