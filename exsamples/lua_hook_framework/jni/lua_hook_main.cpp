#include <stdio.h>
#include <stdlib.h>
#include <utils/PrintLog.h>
#include <string>
#include "hook_frame/exception_hook.h"
#include "system/system.h"
#include "lua_hook/lua_hook_imp.h"

extern "C"{

int lua_hook_main()
{
	LOGD("lua_hook_main : Inject_entry Func is called\n");

	// process name : com.ls.smblgz.uc
	// module : libulua.so
	// function: luaL_loadfile, luaL_loadfilex, luaL_loadbuffer, luaL_loadbufferx
	std::string lib_name = "libulua.so";
	std::string func_name = "luaL_loadbuffer";

	static LuaHookFrame lua_hook_frame_obj;
	lua_hook_frame_obj.InstallHook(lib_name);

	return 5;
}
}
