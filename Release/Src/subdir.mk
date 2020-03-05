################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/BMS.c \
../Src/DHAB_s125.c \
../Src/DMA_USART.c \
../Src/LTC6804.c \
../Src/adc.c \
../Src/can.c \
../Src/dma.c \
../Src/dwt_stm32_delay.c \
../Src/eeprom.c \
../Src/flash.c \
../Src/gpio.c \
../Src/main.c \
../Src/nextion.c \
../Src/nextion_functions.c \
../Src/spi.c \
../Src/stm32f1xx_hal_msp.c \
../Src/stm32f1xx_it.c \
../Src/system_stm32f1xx.c \
../Src/tim.c \
../Src/usart.c 

OBJS += \
./Src/BMS.o \
./Src/DHAB_s125.o \
./Src/DMA_USART.o \
./Src/LTC6804.o \
./Src/adc.o \
./Src/can.o \
./Src/dma.o \
./Src/dwt_stm32_delay.o \
./Src/eeprom.o \
./Src/flash.o \
./Src/gpio.o \
./Src/main.o \
./Src/nextion.o \
./Src/nextion_functions.o \
./Src/spi.o \
./Src/stm32f1xx_hal_msp.o \
./Src/stm32f1xx_it.o \
./Src/system_stm32f1xx.o \
./Src/tim.o \
./Src/usart.o 

C_DEPS += \
./Src/BMS.d \
./Src/DHAB_s125.d \
./Src/DMA_USART.d \
./Src/LTC6804.d \
./Src/adc.d \
./Src/can.d \
./Src/dma.d \
./Src/dwt_stm32_delay.d \
./Src/eeprom.d \
./Src/flash.d \
./Src/gpio.d \
./Src/main.d \
./Src/nextion.d \
./Src/nextion_functions.d \
./Src/spi.d \
./Src/stm32f1xx_hal_msp.d \
./Src/stm32f1xx_it.d \
./Src/system_stm32f1xx.d \
./Src/tim.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F103xB -I"C:/Users/hngui/Documents/GitHub/BMS_2019/Inc" -I"C:/Users/hngui/Documents/GitHub/BMS_2019/Drivers/STM32F1xx_HAL_Driver/Inc" -I"C:/Users/hngui/Documents/GitHub/BMS_2019/Drivers/STM32F1xx_HAL_Driver/Inc/Legacy" -I"C:/Users/hngui/Documents/GitHub/BMS_2019/Drivers/CMSIS/Device/ST/STM32F1xx/Include" -I"C:/Users/hngui/Documents/GitHub/BMS_2019/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


