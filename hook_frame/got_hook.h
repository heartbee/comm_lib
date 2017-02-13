#ifndef GOT_HOOK_H_
#define GOT_HOOK_H_
#include <string>

class GotHookFrame {
public:
	GotHookFrame();
	~GotHookFrame();

	// void *symbol, void *new_function, void **old_function
	bool install_hook(const std::string &so_path, size_t symbol, uint32_t hook_func_addr, uint32_t & org_func_addr);
};

#endif
