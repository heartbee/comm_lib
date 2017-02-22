#ifndef EXCEPTION_HOOK_H_
#define EXCEPTION_HOOK_H_

#include <string>
#include <map>
#include <vector>
#include <memory>
#include<iostream>
#include <asm/siginfo.h>

class ExceptionHandle{
public:
	ExceptionHandle() {

	}
	virtual ~ExceptionHandle() {

	}

public:
	virtual void exception_handle(int sig_num, siginfo_t *sig_info, void *context) = 0;
};

// struct siginfo_t;
class HookInsnStruct{
public:
	bool is_cur_thumb_;
	uint32_t cur_insn_;
	bool is_next_thumb_;
	uint32_t next_insn_;

public:
	HookInsnStruct(bool is_cur_thumb, uint32_t cur_insn, bool is_next_thumb, uint32_t next_insn) :
		is_cur_thumb_(is_cur_thumb), cur_insn_(cur_insn), is_next_thumb_(is_next_thumb), next_insn_(next_insn){
	}

	HookInsnStruct() {
		memset(this, 0, sizeof(HookInsnStruct));
	}
};

class ExceptionHook {
	static std::auto_ptr<ExceptionHook> instance_;

protected:
	ExceptionHook();

public:
	~ExceptionHook();
    //Return this singleton class' instance pointer
    static ExceptionHook* Instance(){
          if(!instance_.get()){
          	instance_ = std::auto_ptr<ExceptionHook>(new ExceptionHook());
          }
          return instance_.get();
    }

	size_t register_exception_handle(std::string &lib_name, size_t addr, ExceptionHandle *handle);
	size_t register_exception_handle(std::string &lib_name, std::string &func_name, ExceptionHandle *handle);

private:
	bool register_exception_handle(size_t exception_addr, ExceptionHandle *handle);
	bool set_exception_hook(size_t addr);
	size_t get_exception_addr(size_t addr);
	static bool write_illegal_insn(size_t addr, bool &is_thumb, uint32_t &org_insn);

	static void call_exception_handle(size_t addr, int sig_num, siginfo_t *sig_info, void *context);
	static void gobal_exception_handle(int signum, siginfo_t *Ssiginfo, void *context);
	static void resume_exception_insn(size_t addr, uint32_t org_insn);
	static int get_insn_length(size_t addr);

private:
	static std::map<size_t, std::vector<ExceptionHandle *> > exception_handle_map_;
	static std::map<size_t, HookInsnStruct > insn_fix_map_;
	bool init_;
};

#endif
