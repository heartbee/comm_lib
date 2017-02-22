################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include obj/local/x86_64/objs/detector_main/system/subdir.mk
-include obj/local/x86_64/objs/detector_main/inject_manager/subdir.mk
-include obj/local/x86_64/objs/detector_main/config/subdir.mk
-include obj/local/x86_64/objs/detector_main/subdir.mk
-include obj/local/x86/objs/detector_main/system/subdir.mk
-include obj/local/x86/objs/detector_main/inject_manager/subdir.mk
-include obj/local/x86/objs/detector_main/config/subdir.mk
-include obj/local/x86/objs/detector_main/subdir.mk
-include obj/local/mips64/objs/detector_main/system/subdir.mk
-include obj/local/mips64/objs/detector_main/inject_manager/subdir.mk
-include obj/local/mips64/objs/detector_main/config/subdir.mk
-include obj/local/mips64/objs/detector_main/subdir.mk
-include obj/local/mips/objs/detector_main/system/subdir.mk
-include obj/local/mips/objs/detector_main/inject_manager/subdir.mk
-include obj/local/mips/objs/detector_main/config/subdir.mk
-include obj/local/mips/objs/detector_main/subdir.mk
-include obj/local/armeabi/objs/detector_main/system/subdir.mk
-include obj/local/armeabi/objs/detector_main/inject_manager/subdir.mk
-include obj/local/armeabi/objs/detector_main/config/subdir.mk
-include obj/local/armeabi/objs/detector_main/subdir.mk
-include obj/local/armeabi-v7a/objs/detector_main/system/subdir.mk
-include obj/local/armeabi-v7a/objs/detector_main/inject_manager/subdir.mk
-include obj/local/armeabi-v7a/objs/detector_main/hook_frame/subdir.mk
-include obj/local/armeabi-v7a/objs/detector_main/elf/subdir.mk
-include obj/local/armeabi-v7a/objs/detector_main/config/subdir.mk
-include obj/local/armeabi-v7a/objs/detector_main/subdir.mk
-include obj/local/arm64-v8a/objs/detector_main/system/subdir.mk
-include obj/local/arm64-v8a/objs/detector_main/inject_manager/subdir.mk
-include obj/local/arm64-v8a/objs/detector_main/config/subdir.mk
-include obj/local/arm64-v8a/objs/detector_main/subdir.mk
-include jni/system/subdir.mk
-include jni/inject_manager/subdir.mk
-include jni/hook_frame/subdir.mk
-include jni/elf/subdir.mk
-include jni/config/subdir.mk
-include jni/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: android_detector_main

# Tool invocations
android_detector_main: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MacOS X C++ Linker'
	g++ -L/Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/lib -o "android_detector_main" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(CPP_DEPS)$(C_DEPS) android_detector_main
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
