LOCAL_PATH := $(call my-dir)  
  
include $(CLEAR_VARS)


LOCAL_MODULE := lua_hook_frame  
LOCAL_SRC_FILES := lua_hook_main.cpp \
                   lua_hook/lua_exception_hook.cpp lua_hook/lua_hook_imp.cpp \
                   system/system.cpp system/dir.cpp system/dbghlp.cpp \
                   hook_frame/exception_hook.cpp \
  
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog  
LOCAL_C_INCLUDES := /Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include/

LOCAL_CFLAGS += -pie -fPIE
LOCAL_LDFLAGS += -pie -fPIE  

include $(BUILD_SHARED_LIBRARY)

