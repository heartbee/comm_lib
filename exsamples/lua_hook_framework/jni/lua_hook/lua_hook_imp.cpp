#include <fstream>
#include "lua_hook_imp.h"
#include "../hook_frame/exception_hook.h"
#include "lua_exception_hook.h"
#include "../system/system.h"
#include <sys/stat.h>
#include<sys/types.h>
#include<dirent.h>
#include <string>
#include "../utils/PrintLog.h"

LuaHookFrame::LuaHookFrame() {
	// install
	lua_exception_hook_obj_ = new LuaExceptionHook(this);


}

LuaHookFrame::~LuaHookFrame() {
	if (NULL != lua_exception_hook_obj_) {
		delete lua_exception_hook_obj_;
	}
}

bool LuaHookFrame::InstallHook(std::string &lib_name) {
	if (NULL == lua_exception_hook_obj_)
		return false;

	std::string func_loadbuffer = "luaL_loadbuffer";
	this->load_buffer_addr_ = ExceptionHook::Instance()->register_exception_handle(lib_name, func_loadbuffer, lua_exception_hook_obj_);


	std::string func_loadbufferx= "luaL_loadbufferx";
	this->load_bufferx_addr_ = ExceptionHook::Instance()->register_exception_handle(lib_name, func_loadbufferx, lua_exception_hook_obj_);

	std::string func_loadfile = "luaL_loadfile";
	this->load_file_addr_ = ExceptionHook::Instance()->register_exception_handle(lib_name, func_loadfile, lua_exception_hook_obj_);

	std::string func_loadfilex = "luaL_loadfilex";
	this->load_filex_addr_ = ExceptionHook::Instance()->register_exception_handle(lib_name, func_loadfilex, lua_exception_hook_obj_);

	return true;
}

int LuaHookFrame::lual_loadbuffer(void* lua_stat, const char * buf,  size_t len, const char * name){
	// mkdir /data/data/tmp
	LOGD("lual_loadbuffere call \n");
	static size_t i = 0;
	char file_name[MAX_FILE_NAME_LEN] = {0};

	if(NULL == opendir("/data/data/tmp"))
	   mkdir("/data/data/tmp",S_IRWXU | S_IRWXG | S_IRWXO);

	if (NULL != name) {
		snprintf(file_name, MAX_FILE_NAME_LEN-1, "/data/data/tmp/buffer_%08x_%s.lua", i++, name);
	}
	else{
		snprintf(file_name, MAX_FILE_NAME_LEN-1, "/data/data/tmp/buffer_%08x.lua", i++);
	}

	std::fstream file;
	file.open(file_name, std::ios::binary|std::ios::out);
	if (!file.is_open()){
		return -1;
	}

	file.write(buf, len);
	file.close();
	return 0;
}

int LuaHookFrame::lual_loadbufferx(void* lua_stat, const char* buf, size_t len, const char* name, const char * mode){
	LOGD("lual_loadbufferx call \n");
	static size_t i = 10000;
	char file_name[MAX_FILE_NAME_LEN] = {0};

	if(NULL == opendir("/data/data/tmp"))
	   mkdir("/data/data/tmp",S_IRWXU | S_IRWXG | S_IRWXO);


	snprintf(file_name, MAX_FILE_NAME_LEN-1, "/data/data/tmp/buffer_%08x.lua", i++);

	std::fstream file;
	file.open(file_name, std::ios::binary|std::ios::out);
	if (!file.is_open()){
		return -1;
	}

	file.write(buf, len);
	file.close();
	return 0;
}

int LuaHookFrame::lual_loadfile(void* lua_stat, const char *name){
	LOGD("lual_loadfile call :%s \n", name);
	return 0;
}
int LuaHookFrame::lual_loadfilex(void* lua_stat, const char *name, const char *mode){
	LOGD("lual_loadfilex call :%s, mode : %s \n", name, mode);
	return 0;
}
