#include "zygote_hook_frame.h"
#include "system/system.h"
#include "hook_frame/exception_hook.h"
#include <utils/PrintLog.h>
#include <sys/mman.h>
#include <dlfcn.h>

std::auto_ptr<ZygoteHookFrame> ZygoteHookFrame::instance_;

ZygoteHookFrame::ZygoteHookFrame() {
	init_succ_ = false;
	fun_dvm_loadNativeCode_base_ = 0;
	fun_dlopen_ = 0;
	init();
}

ZygoteHookFrame::~ZygoteHookFrame() {

}

void ZygoteHookFrame::exception_handle(int sig_num, siginfo_t *sig_info, void *context) {
	ucontext_t *ucontext = (ucontext_t *)context;
	struct sigcontext sigcontext = *(struct sigcontext *)&(ucontext->uc_mcontext);
	LOGD("ZygoteHookFrame exception handle call %08x target1 : %08x, target2 : %08x\n", sigcontext.arm_pc, fun_dvm_loadNativeCode_base_, fun_dlopen_);

	if (sigcontext.arm_pc  == fun_dvm_loadNativeCode_base_) {
		dvmLoadNativeCode((char *)sigcontext.arm_r0, (void *)sigcontext.arm_r1, (char **)sigcontext.arm_r2);
	}
	if (sigcontext.arm_pc == fun_dlopen_) {
		my_dlopen((char*)sigcontext.arm_r0, sigcontext.arm_r1);
	}
	return;
}

void ZygoteHookFrame::init() {
	if (init_succ_) {
		LOGD("ZygoteHookFrame already init.");
		return;
	}
	// if in zygote process
	if (!in_zygote_process()) {
		LOGD("not in zygote process .. ");
		return;
	}
	LOGD("in zygote process ");

	// if (!hook_dvmLoad()) {
	//	return;
	// }
	if (!hook_dlopen()) {
		return;
	}
	init_succ_ = true;
}

bool ZygoteHookFrame::in_zygote_process() {
	std::string process_name;
	SystemInterface::get_process_name (-1, process_name);
	if (process_name.find("zygote") != std::string::npos) {
		// enum process modules
		LOGD("in zygote process . ");
		std::vector<ModuleInfo> module_list;
		if (SystemInterface::enum_modules(-1, module_list)) {
			for (std::vector<ModuleInfo>::iterator itr = module_list.begin(); itr != module_list.end(); ++itr) {
				// LOGD("zygote module : %s ", itr->name.c_str());
			}
		}
		return true;
	}
	LOGD("not in zygote process ");
	return false;
}

bool ZygoteHookFrame::add_inject(std::string &process_name, std::string& lib_name){
	if (inject_map_.find(process_name) != inject_map_.end()){
		std::set<std::string> tmp_lib_set = inject_map_[process_name];
		tmp_lib_set.insert(lib_name);
		inject_map_[process_name] = tmp_lib_set;
		return true;
	}
	else {
		std::set<std::string> tmp_lib_set;
		tmp_lib_set.insert(lib_name);
		inject_map_.insert(std::make_pair(process_name, tmp_lib_set));
	}
	return true;;
}

bool ZygoteHookFrame::hook_dlopen() {
	std::string liblink_name = "/system/bin/linker";
	std::string dl_open_name = "__dl_dlopen";
	size_t dl_open_offset = 0x0e98;
	size_t dl_open_addr = (size_t)dlopen;
	size_t linker_base = SystemInterface::get_module_base(-1, liblink_name);

	LOGD("hook_dlopen dl_open_addr : %08x, linker_base : %08x", dl_open_addr, linker_base);
	fun_dlopen_ = ExceptionHook::Instance()->register_exception_handle(liblink_name, 0x0e51, this);
	if (-1 == fun_dlopen_ || 0 == fun_dlopen_) {
		LOGD("install dlopen hook failed ");
		return false;
	}
	LOGD("install dlopen hook success fun_dlopen : %08x ", fun_dlopen_);
	// print dl_open ins
	// printf("after install hook opcode : %08x ", *insn);
	return true;
}

void* ZygoteHookFrame::my_dlopen(char* soPath, unsigned int mode) {
	// get current process id and name
	static int x = 0;
	if (x <= 1000){
		pid_t pid = getpid();
		LOGD("dlopen : process id : %08x, name : %s, mode : %08x \n", pid, soPath, mode);
	}
	return NULL;
}

bool ZygoteHookFrame::hook_dvmLoad() {
	// get _Z17dvmLoadNativeCodePKcP6ObjectPPc function address and set hook
	std::string libdvm_name = "/system/lib/libdvm.so";
	std::string dvm_load_name = "_Z17dvmLoadNativeCodePKcP6ObjectPPc";
	fun_dvm_loadNativeCode_base_ = ExceptionHook::Instance()->register_exception_handle(libdvm_name, dvm_load_name, this);
	if (-1 == fun_dvm_loadNativeCode_base_ || 0 == fun_dvm_loadNativeCode_base_) {
		LOGD("install _Z17dvmLoadNativeCodePKcP6ObjectPPc hook failed ");
		return false;
	}
	LOGD("install _Z17dvmLoadNativeCodePKcP6ObjectPPc hook success ");

	//system/bin/linker
	//__dl_dlopen

	return true;
}

bool ZygoteHookFrame::dvmLoadNativeCode(char* soPath, void* classLoader, char** detail) {
	LOGD("ZygoteHookFrame dvmLoadNativeCode: soPath : %s, classloader : %08x, detail : %08x \n", soPath, classLoader, detail);
	return true;
}


