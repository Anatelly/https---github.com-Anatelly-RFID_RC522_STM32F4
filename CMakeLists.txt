# Укажем под какую систему мы все нижеследующее будем собирать
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.17)

# Укажим какие компиляторы линковщики и т.д. будем использовать для кросс-компиляции
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Настройки проекта
project(RFID C CXX ASM)
set(CMAKE_CXX_STANDART 17)
set(CMAKE_C_STANDART 99)

# Раскоментируйте ниже строчки для использования аппаратного FPU
#add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
#add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
#add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

add_compile_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
	message(STATUS "Maximum optimization for speed")
	add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
	message(STATUS "Maximum optimization for speed, debug info included")
	add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
	message(STATUS "Maximum optimization for size")
	add_compile_options(-Os)
else ()
	message(STATUS "Minimal optimization, debug info included")
	add_compile_options(-Og -g)
endif ()

# Тут прописываем пути ко всем инклудам проекта
include_directories(
		Core/Inc
		Drivers/STM32F4xx_HAL_Driver/Inc
		Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
		Drivers/CMSIS/Include
		Drivers/CMSIS/Device
		Drivers/iUnilib
		Drivers/iUnilib/common
		Drivers/iUnilib/crc
		Drivers/iUnilib/Interface
		Drivers/iUnilib/Interface/interface_modules/uart_device
)

# Тут прописываем дополнительные дефайны к проекту
add_definitions(-DUSE_HAL_DRIVER -DSTM32F411xE -DHSE_VALUE=16000000 -DF_CPU=72000000)

# Тут прописываются пути ко всем папкам с исходниками
set(GROUP_SOURCES_CMSIS
		Drivers/CMSIS/Device/system_stm32f4xx.c
		Drivers/CMSIS/Device/startup_stm32f411xe.s
		)

set(GROUP_SOURCES_CORE
		Core/Src/main.c
		Core/Src/RFID_module.c
		Core/Src/rc522.c
		Core/Src/stm32f4xx_it.c
		Core/Src/low_level.c
		Core/Src/lock.c
		)

set(GROUP_SOURCES_HAL_DRIVER
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c
		Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c
		)

set(GROUP_SOURCES_LIBS
		Drivers/iUnilib/common/led.c
		Drivers/iUnilib/common/delay.c
		Drivers/iUnilib/common/fifo.c
		Drivers/iUnilib/common/software_timer.c
		Drivers/iUnilib/Interface/interface_modules/uart_device/uart_it.c
		Drivers/iUnilib/crc/crc8.c
		Drivers/iUnilib/crc/crc7.c
		Drivers/iUnilib/crc/crc16_xmodem.c
		)

set(GROUP_SOURCES_INTERFACE
		Drivers/iUnilib/Interface/interface.c
		Drivers/iUnilib/Interface/interface_collector.c
		Drivers/iUnilib/Interface/interface_modules/uart_device/uart_it.c
		Drivers/iUnilib/Interface/interface_modules/uart_device/uart_device.c
		)

set(SOURCES
		${GROUP_SOURCES_CMSIS}
		${GROUP_SOURCES_CORE}
		${GROUP_SOURCES_HAL_DRIVER}
		${GROUP_SOURCES_LIBS}
		${GROUP_SOURCES_INTERFACE}
		${GROUP_SOURCES_FATFS}
		${GROUP_SOURCES_SDIO}
		)

# Тут указывается файл скрипта линковщика
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/STM32F411RE_FLASH.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_link_options(-u _printf_float)
add_link_options(--specs=nano.specs)
add_link_options(-T ${LINKER_SCRIPT})


add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

target_link_libraries(${PROJECT_NAME}.elf
		m
		c
		nosys)

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
		COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
		COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
		COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")

