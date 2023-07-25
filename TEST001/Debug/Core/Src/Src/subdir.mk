################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Src/usr_debug_tool.c \
../Core/Src/Src/usr_isr_callback.c \
../Core/Src/Src/usr_log.c \
../Core/Src/Src/usr_main.c 

OBJS += \
./Core/Src/Src/usr_debug_tool.o \
./Core/Src/Src/usr_isr_callback.o \
./Core/Src/Src/usr_log.o \
./Core/Src/Src/usr_main.o 

C_DEPS += \
./Core/Src/Src/usr_debug_tool.d \
./Core/Src/Src/usr_isr_callback.d \
./Core/Src/Src/usr_log.d \
./Core/Src/Src/usr_main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Src/%.o Core/Src/Src/%.su Core/Src/Src/%.cyclo: ../Core/Src/Src/%.c Core/Src/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F401xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/ST/git/STM32-NucleoF401RE/TEST001/Core/Inc/Inc" -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Src

clean-Core-2f-Src-2f-Src:
	-$(RM) ./Core/Src/Src/usr_debug_tool.cyclo ./Core/Src/Src/usr_debug_tool.d ./Core/Src/Src/usr_debug_tool.o ./Core/Src/Src/usr_debug_tool.su ./Core/Src/Src/usr_isr_callback.cyclo ./Core/Src/Src/usr_isr_callback.d ./Core/Src/Src/usr_isr_callback.o ./Core/Src/Src/usr_isr_callback.su ./Core/Src/Src/usr_log.cyclo ./Core/Src/Src/usr_log.d ./Core/Src/Src/usr_log.o ./Core/Src/Src/usr_log.su ./Core/Src/Src/usr_main.cyclo ./Core/Src/Src/usr_main.d ./Core/Src/Src/usr_main.o ./Core/Src/Src/usr_main.su

.PHONY: clean-Core-2f-Src-2f-Src

