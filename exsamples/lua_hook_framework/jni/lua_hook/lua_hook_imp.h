#include "lua_exception_hook.h"
#include "lua_hook_internal.h"

class LuaHookFrame : public LuaHookInternal{
public:
	LuaHookFrame();
	~LuaHookFrame();

public:
	bool InstallHook(std::string &lib_name);

	virtual int lual_loadbuffer(void* lua_stat, const char * buf,  size_t len, const char * name);
	virtual int lual_loadbufferx(void* lua_stat, const char* buf, size_t len, const char* name, const char * mode);

	virtual int lual_loadfile(void* lua_stat, const char *name);
	virtual int lual_loadfilex(void* lua_stat, const char *name, const char *mode);

private:
	LuaExceptionHook * lua_exception_hook_obj_;

};
