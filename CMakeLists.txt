cmake_minimum_required(VERSION 3.16)

# Project configuration
project(STM32_Router C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Target configuration
set(TARGET STM32_Router)

# STM32F745VETx specific configuration
set(STM32_FAMILY STM32F7xx)
set(STM32_DEVICE STM32F745xx)
set(CPU cortex-m7)
set(FPU fpv5-d16)
set(FLOAT_ABI hard)

# Toolchain configuration
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Find toolchain
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
find_program(ARM_NONE_EABI_GDB arm-none-eabi-gdb)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)

if(NOT ARM_NONE_EABI_GCC)
    message(FATAL_ERROR "arm-none-eabi-gcc not found! Please install ARM GCC toolchain.")
endif()

# Set compilers
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${CPU} -mthumb -mfpu=${FPU} -mfloat-abi=${FLOAT_ABI}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${STM32_DEVICE} -DUSE_HAL_DRIVER")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=${CPU} -mthumb -mfpu=${FPU} -mfloat-abi=${FLOAT_ABI}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${CMAKE_SOURCE_DIR}/STM32F745VETx_FLASH.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -Wl,-Map=${TARGET}.map")

# Include directories
include_directories(
    Core/Inc
    Drivers/CMSIS/Device/ST/STM32F7xx/Include
    Drivers/CMSIS/Include
    Drivers/STM32F7xx_HAL_Driver/Inc
    include
)

# Source files
set(HAL_SOURCES
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_exti.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
    Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
)

set(CORE_SOURCES
    Core/Src/main.c
    Core/Src/stm32f7xx_hal_msp.c
    Core/Src/stm32f7xx_it.c
    Core/Src/system_stm32f7xx.c
    Core/Startup/startup_stm32f745vetx.s
)

set(APP_SOURCES
    src/_Main.c
    src/ARINC.c
)

# Create executable
add_executable(${TARGET}.elf
    ${HAL_SOURCES}
    ${CORE_SOURCES}
    ${APP_SOURCES}
)

# Linker script (you may need to adjust this path)
set(LINKER_SCRIPT STM32F745VETx_FLASH.ld)

# Check if linker script exists, otherwise create a basic one
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
    message(WARNING "Linker script ${LINKER_SCRIPT} not found! Creating a basic one.")
    set(LINKER_SCRIPT ${CMAKE_BINARY_DIR}/${LINKER_SCRIPT})
endif()

target_link_libraries(${TARGET}.elf -T${LINKER_SCRIPT})

# Create hex and bin files
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
    COMMAND ${ARM_NONE_EABI_OBJCOPY} -O ihex ${TARGET}.elf ${TARGET}.hex
    COMMAND ${ARM_NONE_EABI_OBJCOPY} -O binary ${TARGET}.elf ${TARGET}.bin
    COMMENT "Creating hex and bin files..."
)

# Print size information
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
    COMMAND arm-none-eabi-size ${TARGET}.elf
    COMMENT "Size information:"
)

# Build types
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")

# Debug flags
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g -O0")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os -DNDEBUG")

# Message
message(STATUS "Building for ${STM32_DEVICE} (${CPU})")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
