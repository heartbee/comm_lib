################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/hook_frame/inline_hook_ex.cpp \
../jni/hook_frame/relocate.cpp 

OBJS += \
./jni/hook_frame/inline_hook_ex.o \
./jni/hook_frame/relocate.o 

CPP_DEPS += \
./jni/hook_frame/inline_hook_ex.d \
./jni/hook_frame/relocate.d 


# Each subdirectory must supply rules for building sources it contributes
jni/hook_frame/%.o: ../jni/hook_frame/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


