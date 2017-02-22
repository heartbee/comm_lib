#include "lua_exception_hook.h"
#include "../utils/PrintLog.h"

LuaExceptionHook::LuaExceptionHook(LuaHookInternal* lua_hook_obj) : lua_hook_obj_(lua_hook_obj) {

}

LuaExceptionHook::~LuaExceptionHook() {

}

void LuaExceptionHook::exception_handle(int sig_num, siginfo_t *sig_info, void *context) {
	// exception handle call
	LOGD("LuaExceptionHook exception handle call \n");

	ucontext_t *ucontext = (ucontext_t *)context;
	struct sigcontext sigcontext = *(struct sigcontext *)&(ucontext->uc_mcontext);

	if (sigcontext.arm_pc  == lua_hook_obj_->load_buffer_addr_) {
		lua_hook_obj_->lual_loadbuffer((void *)sigcontext.arm_r0, (const char*)sigcontext.arm_r1, sigcontext.arm_r2, (const char *)sigcontext.arm_r3);
	}
	else if(sigcontext.arm_pc  == lua_hook_obj_->load_bufferx_addr_) {
		// 最后一个参数，使用sp传递
		const char ** mode_name_base = (const char **)(sigcontext.arm_sp + 4);
		lua_hook_obj_->lual_loadbufferx((void *)sigcontext.arm_r0, (const char*)sigcontext.arm_r1, sigcontext.arm_r2, (const char *)sigcontext.arm_r3, *mode_name_base);
	}
	else if (sigcontext.arm_pc == lua_hook_obj_->load_file_addr_) {
		lua_hook_obj_->lual_loadfile((void *)sigcontext.arm_r0, (const char*)sigcontext.arm_r1);
	}
	else if(sigcontext.arm_pc == lua_hook_obj_->load_filex_addr_){
		lua_hook_obj_->lual_loadfilex((void *)sigcontext.arm_r0, (const char*)sigcontext.arm_r1, (const char*)sigcontext.arm_r2);
	}
	return;
}
