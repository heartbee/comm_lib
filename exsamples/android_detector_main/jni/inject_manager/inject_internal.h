#ifndef INJECT_INTERNAL_H_
#define INJECT_INTERNAL_H_

#include <string>

class InjectDynamicInternal{
public:
	virtual bool inject(pid_t pid, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params) = 0;

public:
	virtual ~InjectDynamicInternal() {

	}
};

class InjectStaticInternal{
public:
	virtual bool inject(std::string& process_name, std::string &lib_path, std::string &func_name, size_t* func_params, size_t num_params) = 0;

public:
	virtual ~InjectStaticInternal() {

	}
};


#endif
