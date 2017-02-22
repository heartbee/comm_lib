#include <asm/ptrace.h>
#include <string>

class Dbghlp{
public:
	Dbghlp(pid_t pid);
	~Dbghlp();

public:
	bool continue_process();
	bool attach_process();
	bool detach_process();
	bool get_process_regs(pt_regs &regs);
	bool set_process_regs(pt_regs &regs);
	bool remote_process_call(size_t pc_ptr, long * params, long num_params, size_t &return_value);
	bool remote_process_sys_call(std::string &lib_name, size_t local_func_addr, long * params, long num_params, size_t &return_value);
	size_t read_process_memory(uint8_t * base, uint8_t * buf, size_t size, bool & may_failed);
	bool write_process_memory(uint8_t *base, uint8_t *buf, size_t size);

	size_t get_remote_func_addr(std::string& module_name, std::string& func_name);
	size_t alloc_remote_process_memory(size_t alloc_size, size_t page_protect);
	size_t load_library(std::string &lib_path, size_t load_flags);

private:
	bool debug_status_;
	pid_t pid_;
	bool regs_need_recover_;
	pt_regs org_regs_;
};
