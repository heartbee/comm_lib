LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)


LOCAL_MODULE := detector_main   
LOCAL_SRC_FILES := detector_main.cpp \
                   config/config_manager.cpp \
                   system/dir.cpp system/system.cpp system/dbghlp.cpp \
                   inject_manager/inject_imp.cpp inject_manager/ptrace_inject.cpp inject_manager/zygote_inject.cpp \
                   elf/elf_inflect.cpp elf/elf32_parse.cpp \
                   hook_frame/inline_hook_ex.cpp hook_frame/relocate.cpp \
                   shellcode.s

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  
LOCAL_C_INCLUDES := /Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include/

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE  

include $(BUILD_EXECUTABLE)

