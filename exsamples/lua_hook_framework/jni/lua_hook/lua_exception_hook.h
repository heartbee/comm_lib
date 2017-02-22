#ifndef LUA_EXCEPTION_H_
#define LUA_EXCEPTION_H_
#include "../hook_frame/exception_hook.h"
#include "lua_hook_internal.h"

class LuaExceptionHook : public ExceptionHandle{
public:
	LuaExceptionHook(LuaHookInternal* lua_hook_obj);
	virtual ~LuaExceptionHook();

public:
	void exception_handle(int sig_num, siginfo_t *sig_info, void *context);

private:
	void dispatch();
	LuaHookInternal * lua_hook_obj_;

};
#endif
