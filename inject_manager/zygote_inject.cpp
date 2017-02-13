#include "zygote_inject.h"
#include <vector>
#include "../system/system.h"

ZygoteInject::ZygoteInject() {

}

ZygoteInject::~ZygoteInject() {

}

bool ZygoteInject::inject(std::string& process_name) {
	bool ret = false;

	std::string zygote_process_name = "zygote";
	std::string zygote_inject_module = "/data/data/libzygote_hook.so";
	std::string start_func_name = "zygote_hook_main";

	SystemInterface::close_selinux();

	std::vector<pid_t> pid_list;
	SystemInterface::enum_system_process(pid_list);
	if (0 == pid_list.size()) {
		printf("enum process failed \n");
		return ret;
	}
	for (std::vector<pid_t>::iterator itr = pid_list.begin(); itr != pid_list.end(); ++itr) {
		std::string tmp_process_name;
		if(SystemInterface::get_process_name(*itr, tmp_process_name)) {
			// printf("process name : %s \n", process_name.c_str());
			if (tmp_process_name == zygote_process_name) {
				printf("find process : %s, and start inject \n", tmp_process_name.c_str());
				PTraceInject::inject(*itr, zygote_inject_module, start_func_name, NULL, 0);
				ret = true;
				break;
			}
		}
	}
	return ret;
}
