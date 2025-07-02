################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Fonts/Fonts.c 

OBJS += \
./Drivers/Fonts/Fonts.o 

C_DEPS += \
./Drivers/Fonts/Fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Fonts/%.o Drivers/Fonts/%.su Drivers/Fonts/%.cyclo: ../Drivers/Fonts/%.c Drivers/Fonts/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Fonts -I../Drivers/LCD -I../Drivers -I../Drivers/Touch -I../Drivers/XPT2046 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Fonts

clean-Drivers-2f-Fonts:
	-$(RM) ./Drivers/Fonts/Fonts.cyclo ./Drivers/Fonts/Fonts.d ./Drivers/Fonts/Fonts.o ./Drivers/Fonts/Fonts.su

.PHONY: clean-Drivers-2f-Fonts

