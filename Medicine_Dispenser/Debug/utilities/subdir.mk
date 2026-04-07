################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utilities/fsl_assert.c 

C_DEPS += \
./utilities/fsl_assert.d 

OBJS += \
./utilities/fsl_assert.o 


# Each subdirectory must supply rules for building sources it contributes
utilities/%.o: ../utilities/%.c utilities/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DCPU_MCXC444VLH -DCPU_MCXC444VLH_cm0plus -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=1 -DCR_INTEGER_PRINTF -DPRINTF_FLOAT_ENABLE=0 -DSDK_DEBUGCONSOLE_UART -DSERIAL_PORT_TYPE_UART=1 -DSDK_OS_FREE_RTOS -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -D__REDLIB__ -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/board" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/source" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/drivers" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/CMSIS" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/CMSIS/m-profile" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/utilities" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/utilities/debug_console/config" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/device" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/device/periph2" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/utilities/debug_console" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/component/serial_manager" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/component/lists" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/utilities/str" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/component/uart" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/freertos/freertos-kernel/include" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/freertos/freertos-kernel/portable/GCC/ARM_CM0" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/freertos/freertos-kernel/template" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/freertos/freertos-kernel/template/ARM_CM0" -I"/home/fookang/NUS/CG2271/Labs Answer/CG2271_git/Project/Medicine_Dispenser/Inc" -O0 -fno-common -g3 -gdwarf-4 -Wall -c -ffunction-sections -fdata-sections -fno-builtin -fmerge-constants -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m0plus -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-utilities

clean-utilities:
	-$(RM) ./utilities/fsl_assert.d ./utilities/fsl_assert.o

.PHONY: clean-utilities

