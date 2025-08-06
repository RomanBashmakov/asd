# Сборка STM32 Router с CMake

## Prerequisites
- ARM GCC Toolchain (arm-none-eabi-gcc)
- CMake 3.16 or higher
- Make or Ninja build system

## Автоматизированная сборка (скриптом)

```bash
# Сделать скрипт сборки исполняемым (только первый раз)
chmod +x build.sh

# Собрать проект
./build.sh
```

## Ручная сборка

```bash
# Создать каталог сборки
mkdir build
cd build

# Конфигурация (сборка Debug)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Сборка
make -j$(nproc)
```

## Выходные файлы сборки
- `STM32_Router.elf` - Отладочный ELF-файл
- `STM32_Router.hex` - Формат Intel HEX для прошивки
- `STM32_Router.bin` - Бинарный формат для прошивки
- `STM32_Router.map` - Карта линковщика (linker map)

## Проверка на ошибки
Сборка не будет успешной при наличии ошибок компиляции. Проверьте вывод на предмет:
- Синтаксических ошибок
- Отсутствующих include-файлов
- Неопределённых символов
- Несоответствий типов

## Прошивка (с использованием OpenOCD)
```bash
openocd -f interface/stlink.cfg -f target/stm32f7x.cfg -c "program STM32_Router.elf verify reset exit"
```

## Конфигурация сборки
- Цель: STM32F745VETx
- Процессор: Cortex-M7
- FLASH: 512KB (0x08000000)
- ОЗУ: 256KB (0x20000000)
- FPU: FPv5-D16
- Плавающая точка: аппаратная (hard)
