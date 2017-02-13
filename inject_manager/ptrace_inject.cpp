#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <utils/PrintLog.h>
#include "ptrace_inject.h"

#include "../base_types.h"
#include "../system/system.h"
#include "../system/dbghlp.h"

PTraceInject::PTraceInject(){
}
PTraceInject::~PTraceInject(){

}

bool PTraceInject::inject(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params){
	// size_t remote_module_addr = 0;
	size_t remote_function_addr = 0;

	// printf("start load library \n");
	/*
	remote_module_addr = SystemInterface::load_library(pid, lib_path, RTLD_NOW| RTLD_GLOBAL);
	if (0 == remote_module_addr){
		printf("load remote module : %s failed \n", lib_path.c_str());
		return false;
	}
	*/

	remote_function_addr = SystemInterface::get_remote_func_addr(pid, lib_path, func_name);
	if (0 == remote_function_addr){
		printf("get remote function address :%s failed \n", func_name.c_str());
		return false;
	}

	size_t return_value = 0;
	if (!SystemInterface::remote_process_call(pid, remote_function_addr, (long * )func_params, num_params, return_value)){
		printf("call remote function failed : %08x \n", remote_function_addr);
		return false;
	}
	printf("call remote function success, return value : %08x \n", return_value);
	return true;
}

bool PTraceInject::inject_shellcode(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params){
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);

	extern uint32_t _dlopen_addr_s, _dlopen_param1_s, _dlopen_param2_s, _dlsym_addr_s, \
			_dlsym_param2_s, _dlclose_addr_s, _inject_start_s, _inject_end_s, _inject_function_param_s, \
			_saved_cpsr_s, _saved_r0_pc_s;

	pt_regs cur_regs = {0};
	pt_regs org_regs = {0};

	std::string linker_path = "/system/bin/linker";
	std::string libc_path = "/system/lib/libc.so";
	size_t remote_buf_base = SystemInterface::alloc_remote_process_memory(pid, 0x4000, PROT_READ | PROT_WRITE | PROT_EXEC);
	if (0 == remote_buf_base)
		return false;

	size_t remote_code_start_ptr = remote_buf_base + 0x3C00;    // 远程进程中存放shellcode代码的起始地址
	size_t local_code_start_ptr = (size_t)&_inject_start_s;     // 本地进程中shellcode的起始地址
	size_t local_code_end_ptr = (size_t)&_inject_end_s;          // 本地进程中shellcode的结束地址

	_dlopen_addr_s = SystemInterface::get_remote_func_addr(pid, linker_path, (size_t)dlopen);
	_dlsym_addr_s = SystemInterface::get_remote_func_addr(pid, libc_path, (size_t) dlsym);
	_dlclose_addr_s = SystemInterface::get_remote_func_addr(pid, linker_path, (size_t) dlclose);

	// 计算shellcode中一些变量的存放起始地址
	size_t code_length = local_code_end_ptr - local_code_start_ptr;

	size_t dlopen_param1_ptr = local_code_start_ptr + code_length + 0x20;
	size_t dlsym_param2_ptr = dlopen_param1_ptr + 260;
	size_t saved_r0_pc_ptr = dlsym_param2_ptr + 260;
	size_t inject_param_ptr = saved_r0_pc_ptr + 260;

		// 写入dlopen的参数LibPath
	strcpy((char *)dlopen_param1_ptr, lib_path.c_str());
	_dlopen_param1_s = GET_REMOTE_ADDR( dlopen_param1_ptr, local_code_start_ptr, remote_code_start_ptr);

	// 写入dlsym的第二个参数，需要调用的函数名称
	strcpy((char *)dlsym_param2_ptr, func_name.c_str() );
	_dlsym_param2_s = GET_REMOTE_ADDR( dlsym_param2_ptr, local_code_start_ptr, remote_code_start_ptr );

	//保存cpsr寄存器
	_saved_cpsr_s = org_regs.ARM_cpsr;

	//保存r0-pc寄存器
	memcpy((void *)saved_r0_pc_ptr, &(org_regs.ARM_r0), 16 * 4 ); // r0 ~ r15
	_saved_r0_pc_s = GET_REMOTE_ADDR( saved_r0_pc_ptr, local_code_start_ptr, remote_code_start_ptr );

	memcpy((void *)inject_param_ptr, func_params, num_params);
	_inject_function_param_s = GET_REMOTE_ADDR( inject_param_ptr, local_code_start_ptr, remote_code_start_ptr );

	if (!SystemInterface::write_process_memory(pid, (uint8_t *)remote_code_start_ptr, (uint8_t *)local_code_start_ptr, 0x400))
		return false;

	size_t return_value = 0;
	return SystemInterface::remote_process_call(pid, remote_code_start_ptr, NULL, 0, return_value);
}
