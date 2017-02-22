#ifndef LUA_HOOK_INTERNAL_H_
#define LUA_HOOK_INTERNAL_H_
class LuaHookInternal{
public:
	virtual int lual_loadbuffer(void* lua_stat, const char * buf,  size_t len, const char * name) = 0;
	virtual int lual_loadbufferx(void* lua_stat, const char* buf, size_t len, const char* name, const char * mode) = 0;

	virtual int lual_loadfile(void* lua_stat, const char *name) = 0;
	virtual int lual_loadfilex(void* lua_stat, const char *name, const char *mode) = 0;

public:
	LuaHookInternal():load_buffer_addr_(0), load_bufferx_addr_(0), load_file_addr_(0), load_filex_addr_(0){
	}
	virtual ~LuaHookInternal() {
	}

public:
	size_t load_buffer_addr_;
	size_t load_bufferx_addr_;
	size_t load_file_addr_;
	size_t load_filex_addr_;
};
#endif
