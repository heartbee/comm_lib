LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)


LOCAL_MODULE := zygote_hook   
LOCAL_SRC_FILES := zygote_hook_main.cpp \
                   config/config_manager.cpp \
                   system/dir.cpp system/system.cpp system/dbghlp.cpp \
                   hook_frame/exception_hook.cpp hook_frame/inline_hook_ex.cpp hook_frame/relocate.cpp \
                   zygote_hook_frame/zygote_hook_frame.cpp \
                   

LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  
LOCAL_C_INCLUDES := /Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include/

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE  

include $(BUILD_SHARED_LIBRARY)

