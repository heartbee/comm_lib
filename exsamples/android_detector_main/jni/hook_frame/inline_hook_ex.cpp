#include "../system/system.h"
#include "inline_hook_ex.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <sys/mman.h>
#include <asm/ptrace.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <elf.h>

#include "relocate.h"
#include "inline_hook_ex.h"
#include "../utils/PrintLog.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define PAGE_START(addr)	(~(PAGE_SIZE - 1) & (addr))
#define SET_BIT0(addr)		(addr | 1)
#define CLEAR_BIT0(addr)	(addr & 0xFFFFFFFE)
#define TEST_BIT0(addr)		(addr & 1)

#define ACTION_ENABLE	0
#define ACTION_DISABLE	1

InlineHookItem::InlineHookItem(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr) {
	init(target_addr, new_addr, proto_addr);
}

InlineHookItem::InlineHookItem(const InlineHookItem &ths) {
	init_success_ = make_copy(ths);
}

bool InlineHookItem::make_copy(const InlineHookItem &ths) {
	target_addr_ = ths.target_addr_;
	new_addr_ = ths.new_addr_;
	proto_addr_ = ths.proto_addr_;
	status_ = ths.status_;
	memcpy(orig_boundaries_, ths.orig_boundaries_, sizeof(orig_boundaries_));
	memcpy(trampoline_boundaries_, ths.trampoline_boundaries_, sizeof(trampoline_boundaries_));
	count_ = ths.count_;
	length_ = ths.length_;

	orig_instructions_ = malloc(length_);
	if (NULL == orig_instructions_) {
		return false;
	}
	memcpy(orig_instructions_, ths.orig_instructions_, length_);
	trampoline_instructions_ = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if (0 == trampoline_instructions_) {
		return false;
	}
	memcpy(trampoline_instructions_, ths.trampoline_instructions_, PAGE_SIZE);
	init_success_ = true;
	return true;
}
InlineHookItem::~InlineHookItem() {
	LOGD("~InlineHookItem call");
	if (NULL != orig_instructions_) {
		free(orig_instructions_);
		orig_instructions_ = NULL;
	}
	if (NULL == trampoline_instructions_) {
		munmap(trampoline_instructions_, PAGE_SIZE);
		trampoline_instructions_ = 0;
	}
}

InlineHookItem InlineHookItem::operator= (const InlineHookItem &ths) {
	if (this != &ths) {
		init_success_= make_copy(ths);
	}
	return *this;
}

void InlineHookItem::init(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr) {
	target_addr_ = target_addr;
	new_addr_ = new_addr;
	proto_addr_ = proto_addr;

	status_ = UNHOOKED;
	init_success_ = false;

	memset(orig_boundaries_, 0, sizeof(orig_boundaries_));
	memset(trampoline_boundaries_, 0, sizeof(trampoline_boundaries_));
	count_ = 0;
	trampoline_instructions_ = NULL;

	length_ = TEST_BIT0(target_addr_) ? 12 : 8;
	orig_instructions_ = malloc(length_);
	if (NULL == orig_instructions_) {
		return;
	}
	memcpy(orig_instructions_, (void *) CLEAR_BIT0(target_addr_), length_);
	trampoline_instructions_ = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if (NULL == trampoline_instructions_) {
		return;
	}
	relocateInstruction(target_addr_, orig_instructions_, length_, trampoline_instructions_, orig_boundaries_, trampoline_boundaries_, &count_);
	init_success_ = true;
}

uint32_t InlineHookItem::get_action_pc(uint32_t cur_pc, uint32_t action) {
	uint32_t offset = 0;
	uint32_t target_pc = 0;

	switch (action){
		case ACTION_ENABLE:
			offset = cur_pc- CLEAR_BIT0(target_addr_);
			for (int i = 0; i < count_; ++i) {
				if (offset == orig_boundaries_[i]) {
					target_pc = (uint32_t) trampoline_instructions_ + trampoline_boundaries_[i];
					LOGD("ACTION ENABLE InlineHookItem get_action_pc, cur_pc : %08x, target_pc : %08x", cur_pc, target_pc);
					break;
				}
			}
			break;

		case ACTION_DISABLE:
			offset = cur_pc - (int) trampoline_instructions_;
			for (int i = 0; i < count_; ++i) {
				if (offset == trampoline_boundaries_[i]) {
					target_pc = CLEAR_BIT0(target_addr_) + orig_boundaries_[i];
					LOGD("ACTION DISABLE InlineHookItem get_action_pc, cur_pc : %08x, target_pc : %08x", cur_pc, target_pc);
					break;
				}
			}
			break;

		default:
			break;
	}
	if (0 == target_pc) {
		// can't find target pc
		LOGD("can't find target pc");
		target_pc = cur_pc;
	}
	return target_pc;
}

bool InlineHookItem::process_thread_pc(pid_t tid, int action) {
	struct pt_regs regs = {0};
	bool ret = false;

	do {
		if (ptrace(PTRACE_GETREGS, tid, NULL, &regs) == 0) {
			LOGD("InlineHookItem process_thread_pc, PTRACE_GETREGS failed");
			break;
		}
		regs.ARM_pc = get_action_pc(regs.ARM_pc, action);
		ptrace(PTRACE_SETREGS, tid, NULL, &regs);
	}while(false);
	ret = true;
	return ret;
}

bool InlineHookItem::lock(int action, pid_t &pid) {
	LOGD("InlineHookItem lock, action : %08x", action);
	size_t count = 0;
	std::vector<pid_t> tids;

	count = SystemInterface::get_threads(getpid(), tids);
	LOGD("InlineHookItem lock, get_threads : %08x", count);
	if (count > 0) {
		pid = fork();
		if (pid == 0) {
			for (size_t i = 0; i < count; ++i) {
				if (ptrace(PTRACE_ATTACH, tids[i], NULL, NULL) == 0) {
					LOGD("InlineHookItem lock in child process, process_thread_pc : %08x", tids[i]);
					waitpid(tids[i], NULL, WUNTRACED);
					process_thread_pc(tids[i], action);
				}
			}
			raise(SIGSTOP);
			// detach thread
			for (size_t i = 0; i < count; ++i) {
				ptrace(PTRACE_DETACH, tids[i], NULL, NULL);
			}
			raise(SIGKILL);
		}
		else if (pid > 0) {
			// while parent process is waiting, set hook in child process
			waitpid(pid, NULL, WUNTRACED);
		}
		else {
			// error
		}
	}
	LOGD("InlineHookItem lock left");
	return true;
}

bool InlineHookItem::unlock(pid_t pid) {
	LOGD("InlineHookItem unlock process : %08x", pid);
	if (pid < 0) {
		return false;
	}
	kill(pid, SIGCONT);
	wait(NULL);
	return true;
}

bool InlineHookItem::unhook()
{
	LOGD("InlineHookItem unhook call");
	pid_t pid = 0;
	if (!lock(ACTION_DISABLE, pid)) {
		LOGD("InlineHooItem unhook lock failed");
		return false;
	}
	do {
		mprotect((void *) PAGE_START(CLEAR_BIT0(target_addr_)), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
		memcpy((void *) CLEAR_BIT0(target_addr_), orig_instructions_, length_);
		mprotect((void *) PAGE_START(CLEAR_BIT0(target_addr_)), PAGE_SIZE * 2, PROT_READ | PROT_EXEC);
		munmap(trampoline_instructions_, PAGE_SIZE);
		trampoline_instructions_ = NULL;
		free(orig_instructions_);
		orig_instructions_ = NULL;
		status_ = UNHOOKED;
		cacheflush(CLEAR_BIT0(target_addr_), CLEAR_BIT0(target_addr_) + length_, 0);
	}while (false);
	unlock(pid);
	return true;
}

bool InlineHookItem::hook()
{
	LOGD("InlineHookItem start hook");
	pid_t pid = 0;
	if (!lock(ACTION_ENABLE, pid)) {
		return false;
	}
	do {

		mprotect((void *) PAGE_START(CLEAR_BIT0(target_addr_)), PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
		if (TEST_BIT0(target_addr_)) {
			size_t i = 0;
			if (CLEAR_BIT0(target_addr_) % 4 != 0) {
				((uint16_t *) CLEAR_BIT0(target_addr_))[i++] = 0xBF00;  // NOP
			}
			((uint16_t *) CLEAR_BIT0(target_addr_))[i++] = 0xF8DF;
			((uint16_t *) CLEAR_BIT0(target_addr_))[i++] = 0xF000;	// LDR.W PC, [PC]
			((uint16_t *) CLEAR_BIT0(target_addr_))[i++] = new_addr_ & 0xFFFF;
			((uint16_t *) CLEAR_BIT0(target_addr_))[i++] = new_addr_ >> 16;
		}
		else {
			((uint32_t *) (target_addr_))[0] = 0xe51ff004;	// LDR PC, [PC, #-4]
			((uint32_t *) (target_addr_))[1] = new_addr_;
		}
		mprotect((void *) PAGE_START(CLEAR_BIT0(target_addr_)), PAGE_SIZE * 2, PROT_READ | PROT_EXEC);

		if (proto_addr_ != NULL) {
			*(proto_addr_) = TEST_BIT0(target_addr_) ? (uint32_t *) SET_BIT0((uint32_t) trampoline_instructions_) : (uint32_t *) trampoline_instructions_;
		}
		status_ = HOOKED;
		cacheflush(CLEAR_BIT0(target_addr_), CLEAR_BIT0(target_addr_) + length_, 0);
	}while(false);
	unlock(pid);
	return true;
}

bool InlineHookEx::register_hook(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr) {
	if (!SystemInterface::is_exec(-1, target_addr) || !SystemInterface::is_exec(-1, new_addr)) {
		LOGD("register_hook target addr : %08x or new_addr : %08x is not execute", target_addr, new_addr);
		return ERROR_NOT_EXECUTABLE;
	}

		std::map<uint32_t, InlineHookItem * > ::iterator itr = registerd_map_.find(target_addr) ;
		if (itr != registerd_map_.end()) {
			LOGD("register_hook target addr : %08x is already registered", target_addr);
			return EERROR_ALREADY_REGISTERED;
		}

		InlineHookItem* item = new InlineHookItem(target_addr, new_addr, proto_addr);
		if (NULL == item) {
			LOGD("register hook alloc InlineHookItem failed");
			return ERROR_OUT_MEMORY;
		}
		registerd_map_.insert(std::make_pair(target_addr, item));
		return ERROR_OK;
}

bool InlineHookEx::hook(uint32_t target_addr, uint32_t new_addr, uint32_t ** proto_addr) {
	uint32_t status = register_hook(target_addr, new_addr, proto_addr) ;
	if (ERROR_OK == status || EERROR_ALREADY_REGISTERED == status) {
		// start hook
		std::map<uint32_t, InlineHookItem * > ::iterator itr = registerd_map_.find(target_addr);
		if (itr == registerd_map_.end()) {
			// error unkown
			LOGD("InlineHookEx hook : Error unkown");
			return false;
		}
		if (itr->second->status_ == HOOKED) {
			LOGD("InlineHookEx hook : Already hooked");
			return false;
		}
		else {
			itr->second->hook();
		}
	}
	return true;
}

bool InlineHookEx::start_hook() {
	std::map<uint32_t, InlineHookItem * > ::iterator itr = registerd_map_.begin();
	for (; itr != registerd_map_.end(); ++itr) {
		if (itr->second->status_ == UNHOOKED) {
			itr->second->hook();
		}
	}
	return true;
}

bool InlineHookEx::unhook(uint32_t target_addr) {
	std::map<uint32_t, InlineHookItem * > ::iterator itr = registerd_map_.find(target_addr) ;
	if (itr != registerd_map_.end()) {
		if (itr->second->status_ == HOOKED) {
			LOGD("InlineHookEx unhook find hooked address : %08x", target_addr);
			itr->second->unhook();
			return true;
		}
	}
	return false;
}

bool InlineHookEx::reset() {
	std::map<uint32_t, InlineHookItem * > ::iterator itr = registerd_map_.begin();
	while (itr !=registerd_map_.end()){
		if (itr->second->status_ == HOOKED) {
			itr->second->unhook();
		}
		delete itr->second;
		registerd_map_.erase(itr++);
	}
	return true;
}
