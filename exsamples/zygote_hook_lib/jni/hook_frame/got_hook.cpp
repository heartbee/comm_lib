#include "got_hook.h"
#include "../elf/elf32_parse.h"
#include "../system/system.h"
#include "../utils/PrintLog.h"

GotHookFrame::GotHookFrame() {

}

GotHookFrame::~GotHookFrame() {

}

bool GotHookFrame::install_hook(const std::string &so_path, size_t symbol, uint32_t hook_func_addr, uint32_t& org_func_addr) {
	Elf32Parse elf32_file(so_path);
	uint32_t got_table_base = 0;
	uint16_t got_table_size = 0;
	if (!elf32_file.parse_got_table(got_table_base, got_table_size, true)) {
		return false;
	}
	size_t so_base = 0;
	std::string so_name = so_path.substr(-1);

	std::string linker_name = "";
	size_t pos = so_path.rfind("/");
	if (pos == std::string::npos || std::string::npos != so_path.find("/system/")) {
		so_name = so_path;
	}
	else {
		so_name = so_path.substr(pos+1);
	}
	so_base = SystemInterface::get_module_base(-1, so_name);

	bool hook_success = false;
	size_t got_count = got_table_size / 4;
	for(size_t i = 0; i < got_count; i = i + 4){
		if(*(uint32_t *)(so_base + got_table_base + i) == (uint32_t)symbol){
			//保存旧值赋新值
			org_func_addr = symbol;
			LOGI("[+] the addr of old_function is: %p",symbol);
			LOGI("[+] the addr of new_function is: %p",hook_func_addr);
			//修改地址写权限，往地址写值
			SystemInterface::write_process_memory(-1, (uint8_t *)so_base + got_table_base + i, (uint8_t*)&hook_func_addr, sizeof(uint32_t));
			LOGI("[+] modify done. it is point to addr : %08x", *(uint32_t *)(so_base + got_table_base + i));
			hook_success = true;
			break;
		}
	}
	if(hook_success){
		LOGI("[+] DoGotHook done");
	}
	return true;
}


/*
int DoGotHook(const char *TargetDir, const char *TargetSoName,
			 void *symbol, void *new_function, void **old_function)
{
	uint32_t uiGotTableStartaddr = 0;
	uint32_t uiGotTableSize = 0;
	//打开要寻找GOT位置的so

	if(NULL == symbol)
	{
		LOGE("[-] symbol is NUll.");
		return -1;
	}

	if(NULL == new_function)
	{
		LOGE("[-] new_function is NUll.");
		return -1;
	}

	if(NULL == TargetDir)
	{
		LOGE("[-] TargetDir is NUll.");
		return -1;
	}

	if(NULL == TargetSoName)
	{
		LOGE("[-] TargetSoName is NUll.");
		return -1;
	}

	char filepath[256] = {0};
	snprintf(filepath, sizeof(filepath), "%s%s",TargetDir, TargetSoName);

	FILE *file = fopen(filepath, "rb");
	if(NULL == file)
	{
		LOGE("[-] DoGotHook open file fail.");
		return -1;
	}
	int nRet = GetGotStartAddrAndSize(file, &uiGotTableStartaddr, &uiGotTableSize);
	if(-1 == nRet)
	{
		LOGE("[-] GetGotStartAddrAndSize fail.");
		return -1;
	}
	LOGI("[+] uiGotTableStartaddr is %08x\n",uiGotTableStartaddr);
	LOGI("[+] uiGotTableSize is %08x\n",uiGotTableSize);

	uint32_t base = get_module_base(-1, TargetSoName);

	int bHaveFoundTargetAddr = 0;
	for(int i = 0; i < uiGotTableSize; i = i + 4)
	{
		if(*(uint32_t *)(base + uiGotTableStartaddr + i) == (uint32_t)symbol)
		{

			//保存旧值赋新值
			*old_function = symbol;

			LOGI("[+] the addr of old_function is: %p",symbol);
			LOGI("[+] the addr of new_function is: %p",new_function);
			//修改地址写权限，往地址写值
			write_data_to_addr(base + uiGotTableStartaddr + i, (uint32_t)new_function);
			LOGI("[+] modify done. it is point to addr : %08x",
				 *(uint32_t *)(base + uiGotTableStartaddr + i));

			bHaveFoundTargetAddr = 1;
			break;
		}
	}
	if(1 == bHaveFoundTargetAddr)
	{
		LOGI("[+] DoGotHook done");
	}
	else
	{
		LOGE("[-] DoGotHook fail");
	}
	fclose(file);
	return 0;
}
*/
