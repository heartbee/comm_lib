#include <set>
#include "inject_internal.h"

class PTraceInject;
class ZygoteInject;

class InjectImp{

public:
	InjectImp();
	~InjectImp();
	bool inject_dynamic(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params);
	bool zygote_inject(std::string& process_name);

private:
	bool register_inject_mode(InjectDynamicInternal * inject_mode);

	std::set<InjectDynamicInternal *> dynamic_inject_set_;

	PTraceInject* ptrace_method_;
	ZygoteInject* zygote_method_;
};
