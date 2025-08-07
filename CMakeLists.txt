cmake_minimum_required(VERSION 3.16)

# Настройка тулчейна
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Конфигурация проекта
project(STM32_Router C ASM)
set(CMAKE_C_STANDARD 11)                  # Используем стандарт C11
set(CMAKE_C_STANDARD_REQUIRED ON)         # Требуем строгую поддержку стандарта

# Настройка цели (имя исполняемого файла)
set(TARGET STM32_Router)

# Конфигурация для STM32F745VETx
set(STM32_FAMILY STM32F7xx)
set(STM32_DEVICE STM32F745xx)
set(CPU cortex-m7)                        # Целевая архитектура
set(FPU fpv5-d16)                         # Используемый FPU
set(FLOAT_ABI hard)                       # Аппаратный вызов для плавающей точки

# Поиск необходимых инструментов в системе
find_program(ARM_NONE_EABI_GCC arm-none-eabi-gcc)
find_program(ARM_NONE_EABI_GDB arm-none-eabi-gdb)
find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)
find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size)
if(NOT ARM_NONE_EABI_SIZE)
    message(FATAL_ERROR "arm-none-eabi-size не найден! Установите соответствующий пакет.")
endif()

# Если компилятор не найден, вывести ошибку и завершить
if(NOT ARM_NONE_EABI_GCC)
    message(FATAL_ERROR "arm-none-eabi-gcc не найден! Установите ARM GCC toolchain.")
endif()

# Устанавливаем компиляторы
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

# Флаги компилятора
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mcpu=${CPU} -mthumb -mfpu=${FPU} -mfloat-abi=${FLOAT_ABI}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D${STM32_DEVICE} -DUSE_HAL_DRIVER")  # Определяем макросы

# Флаги компоновщика
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mcpu=${CPU} -mthumb -mfpu=${FPU} -mfloat-abi=${FLOAT_ABI} -Wl,--gc-sections -Wl,-Map=${TARGET}.map")
# -Wl,--gc-sections - удаляет неиспользуемые секции, -Map генерирует файл карты памяти

# HAL источники
file(GLOB HAL_SOURCES
    Drivers/STM32F7xx_HAL_Driver/Src/*.c
)

# Core источники
file(GLOB CORE_SOURCES
    Core/Src/*.c
    Core/Startup/*.s
)

# Источники приложения
file(GLOB APP_SOURCES
    Src/*.c
)

# Источники Libs/tools
set(TOOLS_SOURCES
    Libs/tools/src/Tool_Common.c
    Libs/tools/src/Tool_HI3220.c
)

# Просто для отобржаения
file(GLOB_RECURSE APP_HEADERS Inc/*.h)

add_executable(${TARGET}.elf
    ${HAL_SOURCES}
    ${CORE_SOURCES}
    ${APP_SOURCES}
    ${TOOLS_SOURCES}
    ${APP_HEADERS}
)

# Пути к заголовкам
target_include_directories(${TARGET}.elf PRIVATE
    Core/Inc
    Drivers/CMSIS/Device/ST/STM32F7xx/Include
    Drivers/CMSIS/Include
    Drivers/STM32F7xx_HAL_Driver/Inc
    Drivers/STM32F7xx_HAL_Driver/Inc/Legacy
    Inc
    Libs/tools/include
)

# Линкер-скрипт — используем старый, проверенный 
set(LINKER_SCRIPT STM32F745VETX_RAM.ld)

# Проверяем наличие файла ld, чтобы избежать ошибок сборки
if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})
    message(FATAL_ERROR "Linker script ${LINKER_SCRIPT} не найден!")
endif()

# Подключаем linker script через параметр компоновщика
target_link_libraries(${TARGET}.elf -T${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT})

# Команда для создания hex и bin файлов после сборки
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
    COMMAND ${ARM_NONE_EABI_OBJCOPY} -O ihex ${TARGET}.elf ${TARGET}.hex
    COMMAND ${ARM_NONE_EABI_OBJCOPY} -O binary ${TARGET}.elf ${TARGET}.bin
    COMMENT "Создание hex и bin файлов..."
)

# Команда для вывода информации о размерах секций после сборки
add_custom_command(TARGET ${TARGET}.elf POST_BUILD
    COMMAND ${ARM_NONE_EABI_SIZE} ${TARGET}.elf
    COMMENT "Информация о размере прошивки:"
)

# Доступные типы сборки
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "DebugQuiet" "Release")

# Определяем базовые флаги компилятора (необходимо)
set(BASE_FLAGS "-mcpu=${CPU} -mthumb -mfpu=${FPU} -mfloat-abi=${FLOAT_ABI} -ffunction-sections -fdata-sections -Wall -Wextra -D${STM32_DEVICE} -DUSE_HAL_DRIVER")

# Начинаем с базовых флагов
set(CMAKE_C_FLAGS "${BASE_FLAGS}")

# Добавляем специфичные флаги для сборок
set(CMAKE_C_FLAGS_DEBUG "-g -O0")
set(CMAKE_C_FLAGS_DEBUGQUIET "-g -O0 -Wno-unused-parameter")
set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG")

# Добавляем флаги по конфигурации
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "DebugQuiet")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUGQUIET}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
else()
    message(WARNING "Unknown build type ${CMAKE_BUILD_TYPE}, defaulting to Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
endif()

# Информационные сообщения
message(STATUS "Сборка для устройства: ${STM32_DEVICE} (${CPU})")
message(STATUS "Тип сборки: ${CMAKE_BUILD_TYPE}")