################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../get_local_IP.c \
../libMUSE.c \
../serializacionMUSE.c 

OBJS += \
./get_local_IP.o \
./libMUSE.o \
./serializacionMUSE.o 

C_DEPS += \
./get_local_IP.d \
./libMUSE.d \
./serializacionMUSE.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


