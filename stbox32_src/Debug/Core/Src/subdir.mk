################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/breakout.c \
../Core/Src/carrace.c \
../Core/Src/game1942.c \
../Core/Src/joystick.c \
../Core/Src/main.c \
../Core/Src/main_menu.c \
../Core/Src/playmariosound.c \
../Core/Src/pong.c \
../Core/Src/snake.c \
../Core/Src/snakedemo.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/tetris.c 

OBJS += \
./Core/Src/breakout.o \
./Core/Src/carrace.o \
./Core/Src/game1942.o \
./Core/Src/joystick.o \
./Core/Src/main.o \
./Core/Src/main_menu.o \
./Core/Src/playmariosound.o \
./Core/Src/pong.o \
./Core/Src/snake.o \
./Core/Src/snakedemo.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/tetris.o 

C_DEPS += \
./Core/Src/breakout.d \
./Core/Src/carrace.d \
./Core/Src/game1942.d \
./Core/Src/joystick.d \
./Core/Src/main.d \
./Core/Src/main_menu.d \
./Core/Src/playmariosound.d \
./Core/Src/pong.d \
./Core/Src/snake.d \
./Core/Src/snakedemo.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/tetris.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Drivers/Fonts -I../Drivers/LCD -I../Drivers -I../Drivers/Touch -I../Drivers/XPT2046 -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/breakout.cyclo ./Core/Src/breakout.d ./Core/Src/breakout.o ./Core/Src/breakout.su ./Core/Src/carrace.cyclo ./Core/Src/carrace.d ./Core/Src/carrace.o ./Core/Src/carrace.su ./Core/Src/game1942.cyclo ./Core/Src/game1942.d ./Core/Src/game1942.o ./Core/Src/game1942.su ./Core/Src/joystick.cyclo ./Core/Src/joystick.d ./Core/Src/joystick.o ./Core/Src/joystick.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/main_menu.cyclo ./Core/Src/main_menu.d ./Core/Src/main_menu.o ./Core/Src/main_menu.su ./Core/Src/playmariosound.cyclo ./Core/Src/playmariosound.d ./Core/Src/playmariosound.o ./Core/Src/playmariosound.su ./Core/Src/pong.cyclo ./Core/Src/pong.d ./Core/Src/pong.o ./Core/Src/pong.su ./Core/Src/snake.cyclo ./Core/Src/snake.d ./Core/Src/snake.o ./Core/Src/snake.su ./Core/Src/snakedemo.cyclo ./Core/Src/snakedemo.d ./Core/Src/snakedemo.o ./Core/Src/snakedemo.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su ./Core/Src/tetris.cyclo ./Core/Src/tetris.d ./Core/Src/tetris.o ./Core/Src/tetris.su

.PHONY: clean-Core-2f-Src

