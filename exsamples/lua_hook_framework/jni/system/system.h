#include <unistd.h>
#include <vector>
#include <string>


const int MAX_FILE_NAME_LEN = 260;
struct ModuleInfo{
public:
	std::string name;
	size_t x_base;
	size_t x_size;
};

class SystemInterface{
public:
	static size_t enum_system_process(std::vector<pid_t> &pid_list);
	static bool get_process_name(pid_t pid, std::string &process_name);
	static bool get_process_name_from_cmdline(pid_t pid, std::string &process_name);
	static bool get_process_name_from_status(pid_t pid, std::string &process_name);
	static size_t enum_modules(pid_t pid, std::vector<ModuleInfo>& module_list);
	static size_t get_module_base(pid_t pid, std::string module_name);

	static size_t alloc_remote_process_memory(pid_t pid, size_t alloc_size, size_t page_protect);
	static size_t load_library(pid_t pid, std::string &lib_path, size_t load_flags);

	static bool remote_process_call(pid_t pid, size_t pc_ptr, long * params, long num_params, size_t &return_value);
	static bool remote_process_sys_call(pid_t pid, std::string &lib_name, size_t local_func_addr, long * params, long num_params, size_t &return_value);

	static size_t read_process_memory(pid_t pid, uint8_t * base, uint8_t * buf, size_t size, bool & may_failed);
	static bool write_process_memory(pid_t pid, uint8_t *base, uint8_t *buf, size_t size);

	static bool virtual_protect(pid_t pid, uint8_t *base, size_t page_protect);

	static size_t get_remote_func_addr(pid_t pid, std::string &module_name, size_t local_func_addr);
	static size_t get_remote_func_addr(pid_t pid, std::string &module_name, std::string &func_name);

	static size_t get_func_addr(pid_t pid, std::string &module_name, std::string &func_name);

};


