
#include <fstream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <asm/user.h>
#include <sys/mman.h>
#include <asm/ptrace.h>
#include <dlfcn.h>
#include <dirent.h>
#include <utils/PrintLog.h>
#include "system.h"
#include "dbghlp.h"

#include "../base_types.h"
#include "dir.h"

bool SystemInterface::is_exec(pid_t pid, uint32_t addr)
{
	bool ret = false;
	FILE *fp = NULL;
	char line[MAX_FILE_NAME_LEN] = {0};
	char file_name[MAX_FILE_NAME_LEN] = {0};
	uint32_t start = 0;
	uint32_t end = 0;
	do {
		// 暂时仅处理当前进程
	    if (pid < 0) {
	        //  枚举自身进程模块
	        snprintf(file_name, MAX_FILE_NAME_LEN-1, "/proc/self/maps");
	    } else {
	        snprintf(file_name, MAX_FILE_NAME_LEN-1, "/proc/%d/maps", pid);
	    }
		fp = fopen(file_name, "r");
		if (fp == NULL) {
			break;
		}
		while (fgets(line, sizeof(line), fp)) {
			if (strstr(line, "r-xp")) {
				start = strtoul(strtok(line, "-"), NULL, 16);
				end = strtoul(strtok(NULL, " "), NULL, 16);
				if (addr >= start && addr <= end) {
					ret = true;
					break;
				}
			}
		}
		fclose(fp);
		break;
	}while(false);
	return ret;
}

size_t SystemInterface::get_threads(pid_t pid, std::vector<pid_t> &tids){
	char dir_path[32] = {0};
	DIR *dir = NULL;
	struct dirent *entry = NULL;
	pid_t tid = 0;

	if (pid < 0) {
		snprintf(dir_path, sizeof(dir_path), "/proc/self/task");
	}
	else {
		snprintf(dir_path, sizeof(dir_path), "/proc/%d/task", pid);
	}
	LOGD("get_threads : %s dir_path", dir_path);
	do {
		dir = opendir(dir_path);
	    if (dir == NULL) {
	    	break;
	    }
	    do {
	    	entry = readdir(dir);
	    	if (NULL == entry) {
	    		break;
	    	}
	    	LOGD("entry->d_name : %s", entry->d_name);
	    	tid = atoi(entry->d_name);

	    	if (tid != 0 && tid != getpid()) {
	    		tids.push_back(tid);
	    	}
	    } while (true);
	    closedir(dir);
	}while(false);
	return tids.size();
}

size_t SystemInterface::enum_system_process(std::vector<pid_t> &pid_list) {
	if (!pid_list.empty())
		pid_list.clear();

    FilePathArr fs;
    if (0 !=enum_files("/proc/", fs, "^[0-9]*$", false, true, EnumFileType_Directory, 0)) {
    	std::vector<std::string>::iterator itr = fs.begin();
    	for (; itr != fs.end(); ++itr) {
    		std::string *tmp_str = itr;

			size_t pos = tmp_str->rfind('/');
			if (std::string::npos == pos)
				continue;
    		pid_t pid = atoi(tmp_str->substr(pos+1).c_str());
    		pid_list.push_back(pid);
    	}
    }

    return pid_list.size();
}

bool SystemInterface::get_process_name(pid_t pid, std::string &process_name) {
	if (get_process_name_from_cmdline(pid, process_name))
		return true;
	return get_process_name_from_status(pid, process_name);
}

bool SystemInterface::get_process_name_from_cmdline(pid_t pid, std::string &process_name){
	char file_name[MAX_FILE_NAME_LEN] = {0};
	char buf[MAX_FILE_NAME_LEN] = {0};

	if (pid == -1) {
		snprintf(file_name, MAX_FILE_NAME_LEN - 1, "/proc/%s/cmdline", "self");
	}
	else {
		snprintf(file_name, MAX_FILE_NAME_LEN - 1, "/proc/%d/cmdline", pid);
	}

	 std::ifstream file(file_name);
	 if (!file.is_open())
		 return false;
	 file.getline (buf, MAX_FILE_NAME_LEN-1);
	 process_name = buf;
	 if (!process_name.empty())
		 return true;
	return false;
}
bool SystemInterface::get_process_name_from_status(pid_t pid, std::string &process_name){
	char file_name[MAX_FILE_NAME_LEN] = {0};
	char buf[MAX_FILE_NAME_LEN] = {0};

	if (pid == -1) {
		snprintf(file_name, MAX_FILE_NAME_LEN - 1, "/proc/%s/status", "self");
	}
	else {
		snprintf(file_name, MAX_FILE_NAME_LEN - 1, "/proc/%d/status", pid);
	}

	 std::ifstream file(file_name);
	 if (!file.is_open())
		 return false;
	 file.getline (buf, MAX_FILE_NAME_LEN-1);
	 process_name = buf;

	 std::string filter_string = "Name:	";
	 size_t pos = process_name.rfind(filter_string.c_str());
	 if (std::string::npos != pos){
		 process_name = process_name.substr(pos+filter_string.size());
	 }
	 return true;
}

size_t SystemInterface::enum_modules(pid_t pid, std::vector<ModuleInfo>& module_list){

	char file_name[MAX_FILE_NAME_LEN] = {0};
	char buf[MAX_FILE_NAME_LEN] = {0};
    if (pid < 0) {
        //  枚举自身进程模块
        snprintf(file_name, MAX_FILE_NAME_LEN-1, "/proc/self/maps");
    } else {
        snprintf(file_name, MAX_FILE_NAME_LEN-1, "/proc/%d/maps", pid);
    }
    std::ifstream file(file_name);
    if (!file.is_open())
    	return 0;
	 // just parse execute module size
	 std::string x_range;

	 std::string x_base;
	 std::string x_end;

	 std::string page_attributes;
	 std::string process_offset;
	 std::string device_num;
	 std::string inode;
	 std::string image_name;

	 while (!file.eof()){
		 file.getline (buf, MAX_FILE_NAME_LEN-1);

		 // parse image line
		 char * p=strtok(buf," \t");
		 // image base
		 if (NULL != p)
			 x_range = p;
		 else
			 continue;

		 p = strtok(NULL," \t");
		 if (NULL != p)
			 page_attributes = p;
		 else
			 continue;

		 p = strtok(NULL," \t");
		 if (NULL != p)
			 process_offset = p;
		 else
			 continue;

		 p = strtok(NULL," \t");
		 if (NULL != p)
			 device_num = p;
		 else
			 continue;

		 p = strtok(NULL," \t");
		 if (NULL != p)
			 inode = p;
		 else
			 continue;

		 p = strtok(NULL," \t");
		 if (NULL != p)
			 image_name = p;
		 else
			 continue;

		 if (std::string::npos == page_attributes.find("x"))
			 continue;

		 size_t pos = x_range.find("-");
		 x_base = x_range.substr(0, pos);
		 x_end = x_range.substr(pos+1);

		 // pack module info
		 ModuleInfo module_info;
		 module_info.name = image_name;
		 module_info.x_base = strtoul(x_base.c_str(), NULL, 16);
		 module_info.x_size = strtoul(x_end.c_str(), NULL, 16) - module_info.x_size;

		 module_list.push_back(module_info);
	 }
	 return module_list.size();
}

size_t SystemInterface::get_module_base(pid_t pid, std::string module_name) {
	std::vector<ModuleInfo> module_list;
	if (0 != enum_modules(pid, module_list)) {
		std::vector<ModuleInfo>::iterator itr = module_list.begin();
		for(; itr != module_list.end(); ++itr) {
			ModuleInfo * module_ptr = itr;
			if(module_ptr->name == module_name)
				return module_ptr->x_base;
		}
	}
	return 0;
}

size_t SystemInterface::get_remote_func_addr(pid_t pid, std::string& module_name, size_t local_func_addr){
	size_t local_module_base = 0;
	size_t remote_module_base = 0;
	size_t remote_func_addr = 0;

	local_module_base = get_module_base(-1, module_name.c_str());
	remote_module_base = get_module_base(pid, module_name.c_str());
	printf("local_module_base : %08x, loca_func_addr : %08x, remote_module_base : %08x \n", local_module_base, local_func_addr, remote_module_base);
	// printf("local_func_addr - local_module_base : %08x \n", local_func_addr - local_module_base);
	remote_func_addr = local_func_addr - local_module_base + remote_module_base;
	return remote_func_addr;
}

size_t SystemInterface::get_remote_func_addr(pid_t pid, std::string& module_name, std::string& func_name) {
	Dbghlp obj(pid);
	return obj.get_remote_func_addr(module_name, func_name);
}

size_t SystemInterface::get_func_addr(pid_t pid, std::string &module_name, std::string &func_name){
	if (pid == -1) {
		size_t module_base = (size_t )dlopen(module_name.c_str(), RTLD_NOW| RTLD_GLOBAL);
		LOGD("get_func_addr module name : %s, module base : %08x", module_name.c_str(), module_base);
		size_t tmp = (size_t)dlsym((void* )module_base, func_name.c_str());
		LOGD("get function addr : %08x, name : %s", tmp, func_name.c_str());
		return (size_t)dlsym((void* )module_base, func_name.c_str());
	}
	else {
		return get_remote_func_addr(pid, module_name, func_name);
	}
}

size_t SystemInterface::alloc_remote_process_memory(pid_t pid, size_t alloc_size, size_t page_protect){
	Dbghlp obj(pid);
	return obj.alloc_remote_process_memory(alloc_size, page_protect);
}

size_t SystemInterface::load_library(pid_t pid, std::string &lib_path, size_t load_flags){
	Dbghlp obj(pid);
	return obj.load_library(lib_path, load_flags);
}

bool SystemInterface::remote_process_call(pid_t pid, size_t pc_ptr, long * params, long num_params, size_t &return_value){
	Dbghlp obj(pid);
	return obj.remote_process_call(pc_ptr, params, num_params, return_value);
}

bool SystemInterface::remote_process_sys_call(pid_t pid, std::string &lib_name, size_t local_func_addr, long * params, long num_params, size_t &return_value){
	Dbghlp obj(pid);
	return obj.remote_process_sys_call(lib_name, local_func_addr, params, num_params, return_value);
}

size_t SystemInterface::read_process_memory(pid_t pid, uint8_t * base, uint8_t * buf, size_t size, bool & may_failed){
	Dbghlp obj(pid);
	return obj.read_process_memory(base, buf, size, may_failed);
}
bool SystemInterface::write_process_memory(pid_t pid, uint8_t *base, uint8_t *buf, size_t size){
	if (-1 == pid) {
		for (size_t i = 0; i < size; ++i) {
			*(uint8_t *)(base+i) = *(buf+i);
		}
	    return true;
	}
	else {
		Dbghlp obj(pid);
		return obj.write_process_memory(base, buf, size);
	}
}

bool SystemInterface::virtual_protect(pid_t pid, uint8_t *buf, size_t buf_size, size_t page_protect) {
	// 暂时仅支持当前进程
	if (-1 == pid) {
		uint32_t page_size = sysconf(_SC_PAGESIZE);
		uint32_t page_start = (uint32_t)buf & (~(page_size-1));
		uint32_t page_count = (buf_size / page_size) + 1;
		for (uint32_t i = 0; i < page_count; ++i) {
			if(-1 ==  mprotect((void *)(page_start + i * page_size), (page_size), page_protect)){
		        LOGE("[-] mprotect error");
		        return false;
		    }
		}
	    return true;
	}
	return false;
}

bool SystemInterface::close_selinux() {
	return true;
}
