################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/zygote_hook_frame/zygote_hook_frame.cpp 

OBJS += \
./jni/zygote_hook_frame/zygote_hook_frame.o 

CPP_DEPS += \
./jni/zygote_hook_frame/zygote_hook_frame.d 


# Each subdirectory must supply rules for building sources it contributes
jni/zygote_hook_frame/%.o: ../jni/zygote_hook_frame/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


