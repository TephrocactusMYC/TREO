#include <util.h>
#include <ree_syscall_number.h>
#include <kernel/pseudo_ta.h>
#include <kernel/user_access.h>
#include <optee_rpc_cmd.h>
#include <kernel/thread.h>
#include <kernel/msg_param.h>
#include <tee/tee_svc.h>
#include <mm/tee_mm.h>
#include <mm/mobj.h>
#include <pta_elf_ta_loader.h>

static int alloc_shared_mobj(size_t size, struct mobj **p_mobj, void **p_va)
{
	void *va;
	struct mobj *mobj = thread_rpc_alloc_payload(size);
	if (!mobj) {
		DMSG("Failed to allocate mobj");
		return -1;
	}
	if (mobj->size < size) {
		DMSG("mobj size too small");
		thread_rpc_free_payload(mobj);
		return -1;
	}
	va = mobj_get_va(mobj, 0, size);
	if (!va) {
		DMSG("Failed to get mobj va");
		thread_rpc_free_payload(mobj);
		return -1;
	}
	*p_mobj = mobj;
	*p_va = va;
	return 0;
}

static int copy_to_shared_mobj(struct mobj **p_mobj, const void *uaddr,
			       size_t size)
{
	struct mobj *obj;
	void *shared_va;
	if (alloc_shared_mobj(size, &obj, &shared_va) != 0) {
		DMSG("Failed to allocate shared mobj");
		return -1;
	}
	if (copy_from_user(shared_va, uaddr, size) != TEE_SUCCESS) {
		DMSG("Failed to copy args from user");
		thread_rpc_free_payload(obj);
		return -1;
	}
	*p_mobj = obj;
	return 0;
}

static int copy_from_shared_mobj(void *p_va, const void *uaddr,
			       size_t size)
{
	if (copy_to_user(uaddr,p_va,size) != TEE_SUCCESS) {
		DMSG("Failed to copy args from user");
		return -1;
	}
	return 0;
}

static int sys_openat(sys_openat_args_t *args)
{
	struct mobj *pathname_mobj;
	if (copy_to_shared_mobj(&pathname_mobj, args->pathname,
				args->pathname_len) != 0) {
		DMSG("Failed to copy pathname to shared mobj!\n");
		return -1;
	}

	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_openat, args->dirfd, args->flags),
		THREAD_PARAM_MEMREF(IN, pathname_mobj, 0, args->pathname_len),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);

	thread_rpc_free_payload(pathname_mobj);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_close(sys_close_args_t *args)
{

	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_close, args->fd, 0),
		THREAD_PARAM_MEMREF(IN, 0, 0, 0),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command\n");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_read(sys_read_args_t *args)
{
	struct mobj *read_buf;
	void *va;
	if (alloc_shared_mobj(args->count,&read_buf,&va) != 0) {
		DMSG("Failed to alloc read_buf!\n");
		return -1;
	}
	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_read, args->fd, 0),
		THREAD_PARAM_MEMREF(IN, read_buf, 0, args->count),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);
	if (copy_from_shared_mobj(va, args->buf,
				args->count) != 0) {
		DMSG("Failed to copy shared mobj to read_buf\n");
		return -1;
	}
	thread_rpc_free_payload(read_buf);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command\n");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_readv(sys_readv_args_t *args)
{
	int res=0;
	for(int i=0;i<args->count;++i){
		sys_read_args_t tmp_arg={
			.fd=args->fd,
			.buf=args->iov[i].iov_base,
			.count=args->iov[i].iov_len,
		};
		int ret=sys_read(&tmp_arg);
		res=res+ret;
	}
	return res;
}

static int sys_write(sys_write_args_t *args)
{
	struct mobj *write_buf;
	if (copy_to_shared_mobj(&write_buf, args->buf,
				args->count) != 0) {
		DMSG("Failed to copy write_buf to shared mobj!\n");
		return -1;
	}
	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_write, args->fd, 0),
		THREAD_PARAM_MEMREF(IN, write_buf, 0, args->count),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);

	thread_rpc_free_payload(write_buf);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command\n");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_pread(sys_pread_args_t *args)
{
	struct mobj *read_buf;
	void *va;
	if (alloc_shared_mobj(args->count,&read_buf,&va) != 0) {
		DMSG("Failed to alloc read_buf!\n");
		return -1;
	}
	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_pread64, args->fd, args->ofs),
		THREAD_PARAM_MEMREF(IN, read_buf, 0, args->count),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);
	if (copy_from_shared_mobj(va, args->buf,
				  args->count) != 0) {
		DMSG("Failed to copy shared mobj to read_buf\n");
		return -1;
	}
	thread_rpc_free_payload(read_buf);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command\n");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_pwrite(sys_pwrite_args_t *args)
{
	struct mobj *write_buf;
	if (copy_to_shared_mobj(&write_buf, args->buf,
				args->count) != 0) {
		DMSG("Failed to copy write_buf to shared mobj!\n");
		return -1;
	}
	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_write, args->fd, args->ofs),
		THREAD_PARAM_MEMREF(IN, write_buf, 0, args->count),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);

	thread_rpc_free_payload(write_buf);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command\n");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_access(sys_access_args_t *args)
{
	struct mobj *pathname_mobj;
	if (copy_to_shared_mobj(&pathname_mobj, args->filename,
				args->pathname_len) != 0) {
		DMSG("Failed to copy filename to shared mobj!\n");
		return -1;
	}

	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_faccessat, args->amode, 0),
		THREAD_PARAM_MEMREF(IN, pathname_mobj, 0, args->pathname_len),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);

	thread_rpc_free_payload(pathname_mobj);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command");
		return -1;
	}

	return params[2].u.value.a;
}

static int sys_lseek(sys_lseek_args_t *args)
{
	struct thread_param params[] = {
		THREAD_PARAM_VALUE(IN, SYS_lseek,args->fd, args->offset),
		THREAD_PARAM_VALUE(IN, args->whence, 0, 0),
		THREAD_PARAM_VALUE(OUT, 0, 0, 0),
	};

	int res = thread_rpc_cmd(OPTEE_RPC_CMD_SYSCALL, ARRAY_SIZE(params),
				 params);

	if (res != TEE_SUCCESS) {
		DMSG("Failed to send RPC command");
		return -1;
	}

	return params[2].u.value.a;
}

static long handle_syscall_impl(long n, void *args)
{
	switch (n) {
	case SYS_openat:
		return (long)sys_openat((sys_openat_args_t *)args);
	case SYS_close:
		return (long)sys_close((sys_close_args_t *)args);
	case SYS_read:
		return (long)sys_read((sys_read_args_t *)args);
	case SYS_readv:
		return (long)sys_readv((sys_readv_args_t *)args);
	case SYS_write:
		return (long)sys_write((sys_write_args_t *)args);
	case SYS_pread64:
		return (long)sys_pread((sys_pread_args_t *)args);
	case SYS_pwrite64:
		return (long)sys_pwrite((sys_pwrite_args_t *)args);
	case SYS_faccessat:
		return (long)sys_access((sys_access_args_t *)args);
	case SYS_lseek:
		return (long)sys_lseek((sys_lseek_args_t *)args);
	default:
		DMSG("Unknown syscall %ld", n);
		return -1;
	}
}

static TEE_Result handle_syscall(uint32_t type, TEE_Param p[TEE_NUM_PARAMS])
{
	long n;
	void *args;

	if (type != TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
				    TEE_PARAM_TYPE_MEMREF_INPUT,
				    TEE_PARAM_TYPE_VALUE_OUTPUT,
				    TEE_PARAM_TYPE_NONE)
	    )
		return TEE_ERROR_BAD_PARAMETERS;

	n = p[0].value.a;
	args = p[1].memref.buffer;

	long res = handle_syscall_impl(n, args);

	reg_pair_from_64(res, &p[2].value.a, &p[2].value.b);

	return TEE_SUCCESS;
}

static TEE_Result invoke_command(void *psess __unused, uint32_t cmd,
				 uint32_t ptypes,
				 TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case PTA_ELF_TA_LOADER_CMD_SYSCALL:
		return handle_syscall(ptypes, params);
	default:
		break;
	}
	return TEE_ERROR_BAD_PARAMETERS;
}

pseudo_ta_register(.uuid = PTA_ELF_TA_LOADER_UUID, .name = "elf_ta_loader.pta",
		   .flags = PTA_DEFAULT_FLAGS,
		   .invoke_command_entry_point = invoke_command);
