#include "inject_imp.h"
#include "ptrace_inject.h"
#include "zygote_inject.h"

InjectImp::InjectImp() : ptrace_method_(NULL), zygote_method_(NULL){
	// register inject methods

	/*
	ptrace_method_ = new(std::nothrow) PTraceInject();
	if (NULL != ptrace_method_) {
		register_inject_mode(ptrace_method_);
	}*/
	zygote_method_ = new(std::nothrow) ZygoteInject();
	if (NULL != zygote_method_){
		register_inject_mode(zygote_method_);
	}
}

InjectImp::~InjectImp() {
	if (NULL != ptrace_method_) {
		delete ptrace_method_;
	}
}

bool InjectImp::register_inject_mode(InjectDynamicInternal * inject_mode){
	std::pair<std::set<InjectDynamicInternal *>::iterator,bool> result;
	result = dynamic_inject_set_.insert(inject_mode);
	return result.second;
}

bool InjectImp::inject_dynamic(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params){
	bool ret = false;
	std::set<InjectDynamicInternal *>::iterator itr = dynamic_inject_set_.begin();
	for (; itr != dynamic_inject_set_.end(); ++itr) {
		if ((*itr)->inject(pid, lib_path, func_name, func_params, num_params)) {
			ret = true;
			break;
		}
	}
	return ret;
}

bool InjectImp::zygote_inject(std::string& process_name) {
	// find process name
	ZygoteInject zygote_inject_obj;
	return zygote_inject_obj.inject(process_name);
}
