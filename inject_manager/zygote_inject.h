#include <string>
#include <asm/ptrace.h>

#include "inject_internal.h"
#include "ptrace_inject.h"

class ZygoteInject: public PTraceInject{
public:
	ZygoteInject();
	virtual ~ZygoteInject();

	bool inject(std::string& process_name);
};
