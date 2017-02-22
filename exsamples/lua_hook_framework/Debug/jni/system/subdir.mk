################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/system/dbghlp.cpp \
../jni/system/dir.cpp \
../jni/system/system.cpp 

OBJS += \
./jni/system/dbghlp.o \
./jni/system/dir.o \
./jni/system/system.o 

CPP_DEPS += \
./jni/system/dbghlp.d \
./jni/system/dir.d \
./jni/system/system.d 


# Each subdirectory must supply rules for building sources it contributes
jni/system/%.o: ../jni/system/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


