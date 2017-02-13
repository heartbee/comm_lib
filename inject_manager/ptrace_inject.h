#ifndef PTRACE_INJECT_H_
#define PTRACE_INJECT_H_
#include <string>
#include <asm/ptrace.h>

#include "inject_internal.h"

class PTraceInject : public InjectDynamicInternal{
public:
	PTraceInject();
	virtual ~PTraceInject();

	virtual bool inject(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params);
	bool inject_shellcode(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params);
};
#endif
