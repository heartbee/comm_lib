#ifndef _INLINE_HOOK_EX_H_
#define _INLINE_HOOK_EX_H_

#include <stdio.h>
#include <map>

enum HOOK_STATUS {
	HOOKED,
	UNHOOKED,
};

enum ERR_STATUS {
	ERROR_UNKNOWN = -1,
	ERROR_OK = 0,
	ERROR_NOT_INITIALIZED,
	ERROR_NOT_EXECUTABLE,
	ERROR_NOT_REGISTERED,
	ERROR_NOT_HOOKED,
	EERROR_ALREADY_REGISTERED,
	ERROR_ALREADY_HOOKED,
	ERROR_SO_NOT_FOUND,
	ERROR_FUNCTION_NOT_FOUND,
	ERROR_OUT_MEMORY
};


class InlineHookItem {
public:
	InlineHookItem(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr);
	InlineHookItem(const InlineHookItem &ths);
	~InlineHookItem();
	InlineHookItem operator= (const InlineHookItem &ths);


public:
	bool hook();
	bool unhook();

	bool lock(int action, pid_t &pid);
	bool unlock(pid_t pid);

private:
	void init(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr);
	bool make_copy(const InlineHookItem &ths);

	bool process_thread_pc(pid_t tid, int action);
	uint32_t get_action_pc(uint32_t cur_pc, uint32_t action);

public:
	uint32_t target_addr_;
	uint32_t new_addr_;
	uint32_t **proto_addr_;
	void *orig_instructions_;
	int orig_boundaries_[4];
	int trampoline_boundaries_[20];
	int count_;
	void *trampoline_instructions_;
	int length_;
	int status_;
	bool init_success_;
};

class InlineHookEx {
public:
	InlineHookEx() {}
	~InlineHookEx() {}

public:
	bool hook(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr);

	bool register_hook(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr);
	bool start_hook();

	bool unhook(uint32_t target_addr);
	bool reset();

private:
	std::map<uint32_t, InlineHookItem *> registerd_map_;
};


//enum ele7en_status registerInlineHook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr);
//enum ele7en_status inlineUnHook(uint32_t target_addr);
//void inlineUnHookAll();
//enum ele7en_status inlineHook(uint32_t target_addr);
//void inlineHookAll();

#endif
