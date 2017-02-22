#include <stdio.h>
#include <vector>
#include <string>

#include "system/system.h"
#include "utils/PrintLog.h"
#include "inject_manager/inject_imp.h"
#include "elf/elf32_parse.h"
#include "hook_frame/inline_hook_ex.h"
#include <dlfcn.h>

typedef uint32_t (*func_dlopen)(char *so_name, int mode);

func_dlopen proto_addr = NULL;

uint32_t my_dl_open(char * so_name, int mode) {
	LOGD("dlopen : %s, mode : %d", so_name, mode);
	return proto_addr(so_name, mode);
}

int main() {
	printf("detector start ... \n");
	// inject com.android.setting
	std::string inject_process_name = "zygote";
	std::string target_process_name = "com.android.settings";
	std::string inject_module_name = "/system/bin/linker";
	std::string inject_module_function = "Inject_entry";

	//elf parse
	//Elf32Parse elf_parse(inject_module_name);
	//elf_parse.main();

	// zygote inject
	InjectImp inject_imp;
	inject_imp.zygote_inject(target_process_name);

	// inline hook test

	/*
	void* base1 = dlopen(inject_module_name.c_str(), RTLD_NOW| RTLD_GLOBAL);
	printf("before linker module base : %08x \n", base1);

	uint32_t dl_open_base = (uint32_t) dlopen;
	uint32_t new_dlopen = (uint32_t) my_dl_open;
	InlineHookEx* inline_hook_obj = new InlineHookEx();
	// uint32_t ** proto_addr = NULL;


	printf("inline_hook : dl_open_base : %08x, new_dlopen: %08x, proto_addr : %08x \n", dl_open_base, new_dlopen, &proto_addr);
	inline_hook_obj->hook(dl_open_base, new_dlopen, (uint32_t **)&proto_addr);
	// inline_hook_obj.hook(dl_open_base, (uint32_t)my_dlopen_func, proto_addr);
	void* base = dlopen(inject_module_name.c_str(), RTLD_NOW| RTLD_GLOBAL);
	printf("after linker module base : %08x \n", base);

	// inline_hook_obj->unhook(dl_open_base);
	// dlopen(inject_module_name.c_str(), RTLD_NOW| RTLD_GLOBAL);
	LOGD("main goto sleep");
	*/

	return 0;
}
