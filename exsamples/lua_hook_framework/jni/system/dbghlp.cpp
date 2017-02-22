#include "dbghlp.h"
#include <sys/ptrace.h>
#include <utils/PrintLog.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include "../base_types.h"
#include "system.h"

Dbghlp::Dbghlp(pid_t pid) {
	pid_ = pid;
	debug_status_ = false;
	regs_need_recover_ = false;

	if (attach_process()) {
		debug_status_ = true;
	}
	if (get_process_regs(org_regs_)) {
		regs_need_recover_ = true;
	}
}

Dbghlp::~Dbghlp(){
	if (regs_need_recover_) {
		set_process_regs(org_regs_);
		regs_need_recover_ = false;
	}
	if (debug_status_) {
		detach_process();
		debug_status_ = false;
	}

}


bool Dbghlp::continue_process(){
	if (ptrace(PTRACE_CONT, pid_, NULL, NULL) < 0){
		printf("continue_process error, pid:%d \n", pid_);
		return false;
	}
	return true;
}

bool Dbghlp::attach_process(){
	if (debug_status_)
		return true;

	int status = 0;
    if (ptrace(PTRACE_ATTACH, pid_, NULL, 0) < 0) {
        printf("attach_process: attach process error, pid:%d \n", pid_);
        return false;
    }
 	// printf("attach process pid:%d \n", pid_);
    waitpid(pid_, &status , WUNTRACED);
    debug_status_ = true;
	return true;
}

bool Dbghlp::detach_process(){
	if (!debug_status_)
		return true;

    if (ptrace(PTRACE_DETACH, pid_, NULL, 0) < 0) {
        printf("detach process error, pid:%d \n", pid_);
        return false;
    }

	// printf("detach process pid:%d \n", pid_);
	debug_status_ = false;
    return true;
}

bool Dbghlp::get_process_regs(pt_regs &regs){
	if (ptrace(PTRACE_GETREGS, pid_, NULL, &regs) < 0){
		printf("get_process_regs error, pid:%d \n", pid_);
		return false;
	}
	return true;
}

bool Dbghlp::set_process_regs(pt_regs &regs){
	if (ptrace(PTRACE_SETREGS, pid_, NULL, &regs) < 0){
		printf("set_process_regs error, pid:%d \n", pid_);
		return false;
	}
	return true;
}

bool Dbghlp::remote_process_call(size_t pc_ptr, long * params, long num_params, size_t& return_value){
	size_t i = 0;
	bool call_success = true;
	pt_regs cur_regs = {0};
	pt_regs org_regs = {0};
	bool regs_need_recover = true;

	// printf("remote process call \n");
	// start call
	// ARM处理器，函数传递参数，将前四个参数放到r0-r3，剩下的参数压入栈中
	get_process_regs(cur_regs);

	for (i = 0; i < num_params && i < 4; i ++) {
		cur_regs.uregs[i] = params[i];
	}
	if (i < num_params) {
		// 分配栈空间，栈的方向是从高地址到低地址
		SP(cur_regs) -= (num_params - i) * sizeof(long);
		if (!write_process_memory((uint8_t *)SP(cur_regs), (uint8_t *)&params[i], (size_t)(num_params - i) * sizeof(long))) {
			return false;
		}
	}

	PC(cur_regs) = pc_ptr;
	// 与BX跳转指令类似，判断跳转的地址位[0]是否为1，如果为1，则将CPST寄存器的标志T置位，解释为Thumb代码
	// 若为0，则将CPSR寄存器的标志T复位，解释为ARM代码
	if (PC(cur_regs) & 1) {
		/* thumb */
		PC(cur_regs) &= (~1u);
		CPSR(cur_regs) |= CPSR_T_MASK;
	} else {
		/* arm */
		CPSR(cur_regs) |= CPSR_T_MASK;
	}
	cur_regs.ARM_lr = 0;
	if (!set_process_regs(cur_regs)|| !continue_process()) {
		printf("ptrace set regs or continue error, pid:%d", pid_);
		return false;
	}
	int stat = 0;
	// 对于使用ptrace_cont运行的子进程，它会在3种情况下进入暂停状态：①下一次系统调用；②子进程退出；③子进程的执行发生错误。
	// 参数WUNTRACED表示当进程进入暂停状态后，立即返回
	// 将ARM_lr（存放返回地址）设置为0，会导致子进程执行发生错误，则子进程进入暂停状态
	waitpid(pid_, &stat, WUNTRACED);
	// 判断是否成功执行函数
	// printf("ptrace call ret status is %d\n", stat);
	while (stat != 0xb7f) {
		if (!continue_process()) {
			printf("ptrace call error");
			call_success = false;
			break;
		}
		waitpid(pid_, &stat, WUNTRACED);
	}
	if (!call_success)
		return false;

	// 获取远程进程的寄存器值，方便获取返回值
	if (!get_process_regs(cur_regs)){
		printf("After call getregs error \n");
		return false;
	}

	return_value = RET(cur_regs);
	return true;
}

bool Dbghlp::remote_process_sys_call(std::string &lib_name, size_t local_func_addr, long * params, long num_params, size_t &return_value){
	size_t remote_func_addr = SystemInterface::get_remote_func_addr(pid_, lib_name, local_func_addr);

	printf("loca function addr : %08x, remote function addr :%08x \n", local_func_addr, remote_func_addr);
	if (!remote_process_call(remote_func_addr, params, num_params, return_value)){
		return false;
	}
	return true;
}

size_t Dbghlp::read_process_memory(uint8_t * base, uint8_t * buf, size_t size, bool & may_failed){
	size_t read_count = 0;
	size_t remain_count = 0;
	uint8_t * src_tmp = base;
	uint8_t * dst_tmp = buf;
	long tmp_buf = 0;
	size_t i = 0;
	bool regs_recover = true;

	read_count = size / sizeof(long);
	remain_count = size % sizeof(long);

	for (i = 0; i < read_count; i ++ ){
		tmp_buf = ptrace(PTRACE_PEEKTEXT, pid_, src_tmp, 0);
		if (-1 == tmp_buf) {
			may_failed = true;
			printf("base : %08x, read_data return -1, may be error, pid:%d", src_tmp, pid_);
		}
		memcpy(dst_tmp, (const void *)(&tmp_buf), sizeof(long));
		src_tmp += sizeof(long);
		dst_tmp += sizeof(long);
	}

	if (remain_count > 0){
		tmp_buf = ptrace(PTRACE_PEEKTEXT, pid_, dst_tmp, 0);
		memcpy(dst_tmp, (const void *)(&tmp_buf), remain_count);
	}
	return size;
}

bool Dbghlp::write_process_memory(uint8_t *base, uint8_t *buf, size_t size){
	size_t write_count = 0;
	size_t remain_count = 0;

	uint8_t * src_tmp = buf;
	uint8_t * dst_tmp = base;

	long tmp_buf = 0;
	size_t i = 0;

	write_count = size / sizeof(long);
	remain_count = size % sizeof(long);

	bool write_success = true;


	// 先讲数据以sizeof(long)字节大小为单位写入到远程进程内存空间中
	for (i = 0; i < write_count; i ++){
		memcpy((void *)(&tmp_buf), src_tmp, sizeof(long));
		// PTRACE_POKETEXT表示从远程内存空间写入一个sizeof(long)大小的数据
		if (ptrace(PTRACE_POKETEXT, pid_, dst_tmp, tmp_buf) < 0){
			printf("Write Remote Memory error, MemoryAddr:0x%lx", (long)dst_tmp);
			return false;
		}
		src_tmp += sizeof(long);
		dst_tmp += sizeof(long);
	}


	// 将剩下的数据写入到远程进程内存空间中
	if (remain_count > 0){
		//先取出原内存中的数据，然后将要写入的数据以单字节形式填充到低字节处
		tmp_buf = ptrace(PTRACE_PEEKTEXT, pid_, dst_tmp, NULL);
		memcpy((void *)(&tmp_buf), src_tmp, remain_count);
		if (ptrace(PTRACE_POKETEXT, pid_, dst_tmp, tmp_buf) < 0){
			printf("Write Remote Memory error, MemoryAddr:0x%lx", (long)dst_tmp);
			return false;
		}
	}
	return true;
}

size_t Dbghlp::get_remote_func_addr(std::string& module_name, std::string& func_name) {
	// 将so库中需要调用的函数名称写入到远程进程内存空间中
	std::string linker_path = "/system/bin/linker";
	size_t remote_module_base = 0;
	size_t remote_func_name_buf = 0;
	size_t page_protect = PROT_READ | PROT_WRITE | PROT_EXEC;
	long params[2] = {0};

	remote_module_base = load_library(module_name, RTLD_NOW| RTLD_GLOBAL);
	if (0 == remote_module_base){
		printf("get_remote_func_addr: load_library failed, %s \n", module_name.c_str());
		return 0;
	}
	printf("get_remote_func_addr: load_library success : %s, base : %08x \n", module_name.c_str(), remote_module_base);

	remote_func_name_buf = alloc_remote_process_memory(func_name.size()+1, page_protect);
	if(!write_process_memory((uint8_t *)remote_func_name_buf, (uint8_t *)func_name.c_str(), func_name.size()+1)) {
		printf("get_remote_func_addr: write_process_memory failed");
		return 0;
	}
	printf("get_remote_func_addr: alloc function name success : %s remote buf : %08x \n", func_name.c_str(), remote_func_name_buf);

	// 设置dlsym的参数，返回值为远程进程内函数的地址
	// void *dlsym(void *handle, const char *symbol);
	params[0] = remote_module_base;
	params[1] = remote_func_name_buf;
	size_t remote_func_addr = 0;
	if (!remote_process_sys_call(linker_path, (size_t)dlsym, params, 2, remote_func_addr)){
		printf("get_remote_func_addr: dlsym call failed \n");
		return 0;
	}
	printf("get_remote_func_addr: dlsym call success, remote_func_addr : %08x \n", remote_func_addr);
	return remote_func_addr;
}

size_t Dbghlp::alloc_remote_process_memory(size_t alloc_size, size_t page_protect){
	std::string libc_path = "/system/lib/libc.so";
	size_t alloc_base = 0;

	long params[6] = {0};
	// 设置mmap的参数
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);
	params[0] = 0;  // 设置为NULL表示让系统自动选择分配内存的地址
	params[1] = alloc_size; // 映射内存的大小
	params[2] = page_protect;  // 表示映射内存区域可读可写可执行
	params[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // 建立匿名映射
	params[4] = 0; //  若需要映射文件到内存中，则为文件的fd
	params[5] = 0; //文件映射偏移量

	// 调用远程进程的mmap函数，建立远程进程的内存映射
	// printf("call remote mmap \n");
	if (!remote_process_sys_call(libc_path, (size_t) mmap, params, 6, alloc_base))
		return 0;
	// LOGD("Remote Process Map Memory Addr:0x%lx", (long)alloc_base);
	return alloc_base;
}

size_t Dbghlp::load_library(std::string &lib_path, size_t load_flags){
	// alloc memory to map lib_path name
	std::string linker_path = "/system/bin/linker";
	size_t page_protect = PROT_READ | PROT_WRITE | PROT_EXEC;
	long params[2] = {0};
	char end = '\0';
	// printf("start alloc remote memory \n");
	size_t remote_name_buf = alloc_remote_process_memory(lib_path.size() + 1, page_protect);
	if (0 == remote_name_buf) {
		printf("load_library: alloc remote buf failed \n");
		return 0;
	}
	// printf("load_library : alloc remote buf : %08x \n", remote_name_buf);
	if (!write_process_memory((uint8_t *)remote_name_buf, (uint8_t *)lib_path.c_str(), lib_path.size()+1)){
		printf("load_library: write process memory failed : %08x \n", remote_name_buf);
		return 0;
	}
	// printf("load_library: write process memory success : %08x \n", remote_name_buf);

	// 设置dlopen的参数,返回值为模块加载的地址
	// void *dlopen(const char *filename, int flag);
	params[0] = remote_name_buf;
	params[1] = load_flags;

	size_t remote_lib_base = 0;
	if(!remote_process_sys_call(linker_path, (size_t) dlopen, params, 2, remote_lib_base)) {
		printf("load_library: remote dlopen call failed \n");
		return 0;
	}
	printf("load_library: dlopen call success, %s base is :%08x \n", lib_path.c_str(), remote_lib_base);
	return remote_lib_base;
}
