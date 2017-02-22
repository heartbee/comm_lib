#include <elf.h>
#include <stddef.h>
#include <ucontext.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <assert.h>
#include "system/system.h"
#include <utils/PrintLog.h>
#include "exception_hook.h"

std::map<size_t, std::vector<ExceptionHandle *> > ExceptionHook::exception_handle_map_;
std::map<size_t, HookInsnStruct > ExceptionHook::insn_fix_map_;
std::auto_ptr<ExceptionHook> ExceptionHook::instance_;

ExceptionHook::ExceptionHook(){
	init_ = false;
	struct sigaction sig_action;
	sigemptyset(&sig_action.sa_mask);
	//设置 SA_SIGINFO ，指定处理函数为sigaction的sa_sigaction函数
	sig_action.sa_flags = SA_SIGINFO;
	sig_action.sa_sigaction = gobal_exception_handle;
	size_t ret = sigaction(SIGILL, &sig_action, NULL);
	if (-1 != ret) {
		init_ = true;
	}
	else{
		LOGE("[-] sigaction error");
	}
}

ExceptionHook::~ExceptionHook() {

}

size_t ExceptionHook::register_exception_handle(std::string &lib_name, size_t exception_offset, ExceptionHandle *handle) {
	size_t exception_addr = SystemInterface::get_module_base(-1, lib_name) + exception_offset;
	size_t module_base = SystemInterface::get_module_base(-1, lib_name);
	LOGD("exception_addr : %08x, exception_offset : %08x, module base : %08x ", exception_addr, exception_offset, module_base);
	if (register_exception_handle(exception_addr, handle))
		return get_exception_addr(exception_addr);
	return -1;
}

size_t ExceptionHook::register_exception_handle(std::string &lib_name, std::string& func_name, ExceptionHandle *handle) {
	size_t exception_addr = SystemInterface::get_func_addr(-1, lib_name, func_name);
	if (exception_addr == 0 || exception_addr == -1) {
		LOGD("find function address failed, lib name: %s, function : %s, addr : %08x", lib_name.c_str(), func_name.c_str(), exception_addr);
		return -1;
	}
	if (register_exception_handle(exception_addr, handle))
		return exception_addr;
	return -1;
}

bool ExceptionHook::register_exception_handle(size_t exception_addr, ExceptionHandle *handle) {
	std::map<size_t, std::vector<ExceptionHandle *> > ::iterator itr = exception_handle_map_.find(exception_addr) ;
	if (itr != exception_handle_map_.end()) {
		exception_handle_map_[exception_addr].push_back(handle);
		return true;
	}

	if (set_exception_hook(exception_addr)) {
		std::vector<ExceptionHandle *> tmp_handle_vector;
		tmp_handle_vector.push_back(handle);
		exception_handle_map_.insert(std::make_pair(exception_addr, tmp_handle_vector));
		return true;
	}
	return false;
}

bool ExceptionHook::set_exception_hook(size_t addr){
	LOGI("[+] DoExceptionHook Writes illegal instruction to addr : %08x ", addr);
	// WriteillegalInstructionAndSaveOpcode(g_uiTargetaddr, &g_uiTargetOriginOpcode);
	if (insn_fix_map_.find(addr) != insn_fix_map_.end()){
		LOGD("1111");
		return true;
	}
	uint32_t org_insn = 0;
	uint32_t next_insn = 0;
	bool is_cur_thumb = false;
	bool is_next_thumb = false;
	if (write_illegal_insn(addr, is_cur_thumb, org_insn)) {
		HookInsnStruct hook_insn_obj(is_cur_thumb, org_insn, is_next_thumb, next_insn);
		insn_fix_map_.insert(std::make_pair(addr, hook_insn_obj));
		LOGD("22222");
		return true;
	}
	LOGD("333");
	return false;
}

size_t ExceptionHook::get_exception_addr(size_t addr){
	if (insn_fix_map_.find(addr) != insn_fix_map_.end()){
		if (insn_fix_map_[addr].is_cur_thumb_) {
			LOGD("is cur_thumb is ture");
			return addr & (~0x00000001);
		}
		else {
			LOGD("cur_thumb is false");
			return addr;
		}
	}
	return 0;
}

bool ExceptionHook::write_illegal_insn(size_t addr, bool &is_thumb, uint32_t &org_insn){
	size_t tmp_base = addr;
	uint32_t illegal_insn = 0;
	if(0x00000001 == (addr & 0x00000001)){
		is_thumb = true;
		org_insn = *(uint32_t *)(addr & (~0x00000001));
		//Thumb illegal instruction : 0xdeXX
		illegal_insn = 0x0000de00 | (0xFFFF0000 & org_insn);
		tmp_base = addr & (~0x00000001);
	}
	else{
		is_thumb = false;
		//Arm illegal instruction: 0xf7fXaXXX
		org_insn = *(uint32_t *)addr;
		illegal_insn = 0x7f000f0;
	}
	SystemInterface::virtual_protect(-1, (uint8_t *)tmp_base, sizeof(uint32_t), PROT_READ | PROT_WRITE | PROT_EXEC);
	SystemInterface::write_process_memory(-1, (uint8_t *)tmp_base, (uint8_t *)&illegal_insn, sizeof(illegal_insn));

	LOGI("[+] is_thumb is %08x \n",is_thumb);
	LOGI("[+] WriteillegalInstruction addr: %08x, OriginalOpcode is %08x",tmp_base, org_insn);
	return true;
}

void ExceptionHook::resume_exception_insn(size_t addr, uint32_t org_insn) {
	SystemInterface::virtual_protect(-1, (uint8_t *)addr, sizeof(uint32_t), PROT_READ | PROT_WRITE | PROT_EXEC);
	SystemInterface::write_process_memory(-1, (uint8_t *)addr, (uint8_t *)&org_insn, sizeof(org_insn));
}

int ExceptionHook::get_insn_length(size_t addr){
	//断言这个是thumb才这样去算
	assert(1 == (addr & 0x00000001));
	int insn_len = 0;
	uint32_t opcode = *(uint32_t *)(addr & (~0x00000001));
	LOGI("[+] opcode is %08x", opcode);
	//只有在[15:13]为 111 以及 [12:11] 不是 00 的时候是 32bit
	if(((opcode & 0x0000E000) == 0x0000E000) && ((opcode & 0x00001800) != 0)){
		// Thumb-2 32bit instruction
		insn_len = 4;
	}
	else{
		insn_len = 2;
	}

	LOGI("[+] get the length of the addr's instruction,length is %08x", insn_len);
	return insn_len;
}

void ExceptionHook::call_exception_handle(size_t addr, int sig_num, siginfo_t *sig_info, void *context) {
	static std::map<size_t, std::vector<ExceptionHandle *> >::iterator handle_list_itr = exception_handle_map_.find(addr);
	if (handle_list_itr != exception_handle_map_.end()) {
		std::vector<ExceptionHandle *> handle_list = exception_handle_map_[addr];
		std::vector<ExceptionHandle *>::iterator handle_itr = handle_list.begin();
		for (; handle_itr != handle_list.end(); ++ handle_itr) {
			ExceptionHandle * handle = (*handle_itr);
			handle->exception_handle(sig_num, sig_info, context);
		}
	}
}

void ExceptionHook::gobal_exception_handle(int sig_num, siginfo_t *sig_info, void *context)
{
	// LOGI("[+] signum is %d", sig_num);
	ucontext_t *ucontext = (ucontext_t *)context;

	//其实下面这个不用转，只不过是因为我的clang不够智能,不转下会提示出错。
	struct sigcontext sigcontext = *(struct sigcontext *)&(ucontext->uc_mcontext);

	std::map<size_t, HookInsnStruct >::iterator itr = insn_fix_map_.begin();
	for(; itr != insn_fix_map_.end(); ++itr){
		// std::pair<size_t, HookInsnStruct> *tmp_pair = &(*itr);
		// thumb
		if (sigcontext.arm_pc == (itr->first & (~0x00000001))){
			// call back in this place
			call_exception_handle(itr->first, sig_num, sig_info, context);
			LOGI("[+] Resume the orign insn, addr : %08x, org_insn : %08x, is_thumb : %d \n", (itr->first & (~0x00000001)), itr->second.cur_insn_, !!itr->second.is_cur_thumb_);
			SystemInterface::write_process_memory(-1, (uint8_t *)(itr->first & (~0x00000001)), (uint8_t *)&itr->second.cur_insn_, sizeof(uint32_t));
			//给下一个地址写非法指令
			int insn_len = 0;
			if(itr->second.is_cur_thumb_)
				insn_len = get_insn_length(itr->first);
			else
				insn_len = 4;
			// write next insn
			write_illegal_insn(itr->first+insn_len, itr->second.is_next_thumb_, itr->second.next_insn_);
			LOGI("[+] write next insn, addr : %08x, org_insn : %08x, is_thumb : %d \n", (itr->first & (~0x00000001))+insn_len, itr->second.next_insn_, !!itr->second.is_next_thumb_);
		}
		else if(sigcontext.arm_pc == ((itr->first & (~0x00000001)) + 2)|| sigcontext.arm_pc == ((itr->first & (~0x00000001)) + 4) ){
			LOGI("next exception addr : %08x", sigcontext.arm_pc);
			resume_exception_insn(sigcontext.arm_pc, itr->second.next_insn_);
			//这里所有指令都恢复正常了
			//重新写非法指令，为下一次目的地址被执行而Hook
			write_illegal_insn(itr->first, itr->second.is_cur_thumb_, itr->second.cur_insn_);
			LOGI("[+] rewrite illegal insn, addr : %08x, org_insn : %08x, is_thumb : %d \n", (itr->first & (~0x00000001)), itr->second.cur_insn_, !!itr->second.is_cur_thumb_);
		}
	}
}
