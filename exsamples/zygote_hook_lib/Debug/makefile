################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include obj/local/armeabi-v7a/objs/zygote_hook/zygote_hook_frame/subdir.mk
-include obj/local/armeabi-v7a/objs/zygote_hook/system/subdir.mk
-include obj/local/armeabi-v7a/objs/zygote_hook/hook_frame/subdir.mk
-include obj/local/armeabi-v7a/objs/zygote_hook/config/subdir.mk
-include obj/local/armeabi-v7a/objs/zygote_hook/subdir.mk
-include jni/zygote_hook_frame/subdir.mk
-include jni/system/subdir.mk
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
all: zygote_hook_lib

# Tool invocations
zygote_hook_lib: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MacOS X C++ Linker'
	g++  -o "zygote_hook_lib" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(CPP_DEPS)$(C_DEPS) zygote_hook_lib
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
