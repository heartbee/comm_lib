################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/inject_manager/elf_inject.cpp \
../jni/inject_manager/inject_imp.cpp \
../jni/inject_manager/ptrace_inject.cpp \
../jni/inject_manager/zygote_inject.cpp 

OBJS += \
./jni/inject_manager/elf_inject.o \
./jni/inject_manager/inject_imp.o \
./jni/inject_manager/ptrace_inject.o \
./jni/inject_manager/zygote_inject.o 

CPP_DEPS += \
./jni/inject_manager/elf_inject.d \
./jni/inject_manager/inject_imp.d \
./jni/inject_manager/ptrace_inject.d \
./jni/inject_manager/zygote_inject.d 


# Each subdirectory must supply rules for building sources it contributes
jni/inject_manager/%.o: ../jni/inject_manager/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


