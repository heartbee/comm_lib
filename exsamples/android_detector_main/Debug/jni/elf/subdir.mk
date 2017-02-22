################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../jni/elf/elf32_parse.cpp \
../jni/elf/elf_base.cpp \
../jni/elf/elf_inflect.cpp \
../jni/elf/elf_object.cpp 

OBJS += \
./jni/elf/elf32_parse.o \
./jni/elf/elf_base.o \
./jni/elf/elf_inflect.o \
./jni/elf/elf_object.o 

CPP_DEPS += \
./jni/elf/elf32_parse.d \
./jni/elf/elf_base.d \
./jni/elf/elf_inflect.d \
./jni/elf/elf_object.d 


# Each subdirectory must supply rules for building sources it contributes
jni/elf/%.o: ../jni/elf/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/Users/chenbinbin/Library/Android/sdk/ndk-bundle/platforms/android-19/arch-arm/usr/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


