#include <stdio.h>
#include <stdlib.h>
#include <utils/PrintLog.h>
#include "system/system.h"
#include "zygote_hook_frame/zygote_hook_frame.h"
#include <sys/mman.h>
#include <dlfcn.h>
#include "hook_frame/inline_hook_ex.h"
#include <string>

extern "C"{

typedef uint32_t (*func_dlopen)(char *so_name, int mode);
typedef int (*func_lua_hook_main)();

func_dlopen proto_addr = NULL;
func_lua_hook_main lua_hook_main = NULL;

static bool g_hooked = false;

std::string lua_hook_lib_name = "/data/data/liblua_hook_frame.so";
std::string lua_hook_main_name = "lua_hook_main";

uint32_t my_dl_open(char * so_name, int mode) {
	LOGD("dlopen : %s, mode : %d", so_name, mode);
	// if include lua
	if (!g_hooked && strstr(so_name, "lua")) {
		LOGD("entry lua hook frame");
		g_hooked = true;
		uint32_t lua_base = proto_addr(so_name, mode);
		// load lua_hook_framework
		uint32_t lua_hook_lib = proto_addr((char*)lua_hook_lib_name.c_str(), RTLD_NOW| RTLD_GLOBAL);
		lua_hook_main = (func_lua_hook_main) dlsym((void*)lua_hook_lib, lua_hook_main_name.c_str());
		int result = lua_hook_main();
		LOGD("lua_base : %08x, lua_hook_lib base : %08x, lua_hook_main base : %08x, result : %d", lua_base, lua_hook_lib, lua_hook_main, result);
		return lua_base;
	}

	return proto_addr(so_name, mode);
}

int zygote_hook_main(){
	LOGD("Inject_entry Func is called\n");
	// read params from config
	/*
	std::string liblink_name = "/system/bin/linker";
	std::string dl_open_name = "__dl__ZL10dlopen_extPKciPK17android_dlextinfo";
	size_t test = SystemInterface::get_func_addr(-1, liblink_name, dl_open_name);
	if (test != 0) {
		uint8_t * insn = (uint8_t *)test;
		LOGD("end install hook opcode : %08x \n", *insn);
	}
	else {
		LOGD("get dlopen failed \n");
	}*/

	//ZygoteHookFrame* zygote_obj = ZygoteHookFrame::Instance();
	InlineHookEx * inline_hook_obj = new InlineHookEx();
	// uint32_t** proto_addr = NULL;
	inline_hook_obj->hook((uint32_t)dlopen, (uint32_t)my_dl_open, (uint32_t **)&proto_addr);
	LOGD("hook end");




	// print

	return 3;
}

}
