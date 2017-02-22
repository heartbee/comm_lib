#ifndef ZYGOTE_HOOK_H_
#define ZYGOTE_HOOK_H_

#include <string>
#include <map>
#include <set>
#include "hook_frame/exception_hook.h"

class ZygoteHookFrame : public ExceptionHandle {
	static std::auto_ptr<ZygoteHookFrame> instance_;
protected:
	ZygoteHookFrame();

public:
	virtual ~ZygoteHookFrame();

    static ZygoteHookFrame* Instance(){
          if(!instance_.get()){
          	instance_ = std::auto_ptr<ZygoteHookFrame>(new ZygoteHookFrame());
          }
          return instance_.get();
    }


	virtual void exception_handle(int sig_num, siginfo_t *sig_info, void *context);

	bool add_inject(std::string &process_name, std::string& lib_name);
private:
	void init();
	bool hook_dvmLoad();
	bool hook_dlopen();
	bool in_zygote_process();

	bool dvmLoadNativeCode(char* soPath, void* classLoader, char** detail);

	void* my_dlopen(char* soPath, unsigned int mode);


private:
	bool init_succ_;
	size_t fun_dvm_loadNativeCode_base_;
	size_t fun_dlopen_;
	std::map<std::string, std::set<std::string> > inject_map_;
};

#endif
