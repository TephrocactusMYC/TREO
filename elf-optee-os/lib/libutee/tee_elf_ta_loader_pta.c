#include <util.h>
#include <string.h>
#include <tee_internal_api.h>
#include <pta_elf_ta_loader.h>

static TEE_Result invoke_pta(uint32_t cmd_id, uint32_t param_types,
			     TEE_Param params[TEE_NUM_PARAMS])
{
	static TEE_TASessionHandle sess = TEE_HANDLE_NULL;
	static const TEE_UUID pta_uuid = PTA_ELF_TA_LOADER_UUID;

	if (sess == TEE_HANDLE_NULL) {
		TEE_Result res = TEE_OpenTASession(
			&pta_uuid, TEE_TIMEOUT_INFINITE, 0, NULL, &sess, NULL);

		if (res != TEE_SUCCESS) {
			return res;
		}
	}

	return TEE_InvokeTACommand(sess, TEE_TIMEOUT_INFINITE, cmd_id,
				   param_types, params, NULL);
}

TEE_Result TEE_forward_syscall(long syscall_id, void *args, size_t size,
			       long *p_ret)
{
	TEE_Result res;
	uint32_t param_types;
	TEE_Param params[TEE_NUM_PARAMS];

	param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
				      TEE_PARAM_TYPE_MEMREF_INPUT,
				      TEE_PARAM_TYPE_VALUE_OUTPUT,
				      TEE_PARAM_TYPE_NONE);

	memset(params, 0, sizeof(params));

	params[0].value.a = syscall_id;
	params[1].memref.buffer = args;
	params[1].memref.size = size;

	res = invoke_pta(PTA_ELF_TA_LOADER_CMD_SYSCALL, param_types, params);

	if (res != TEE_SUCCESS) {
		return res;
	}

	if (p_ret) {
		*p_ret = reg_pair_to_64(params[2].value.a, params[2].value.b);
	}
	return TEE_SUCCESS;
}
