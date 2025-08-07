#!/bin/bash

# Скрипт сборки для проекта STM32 Router

#Папка с результатами
BUILD_DIR="build"

# Путь к тулчейну
TOOLCHAIN_PATH="/usr/local/bin"  

#Необходимые пакеты
REQUIRED_PKGS=("cmake" "make" "arm-none-eabi-gcc")

# Проверка необходимых пакетов
MISSING_PKGS=()
for pkg in "${REQUIRED_PKGS[@]}"; do
    if ! command -v "$pkg" >/dev/null 2>&1; then
        MISSING_PKGS+=("$pkg")
    fi
done

if [ ${#MISSING_PKGS[@]} -ne 0 ]; then
    echo "Ошибка: не найдены необходимые пакеты:"
    for pkg in "${MISSING_PKGS[@]}"; do
        echo "  - $pkg"
    done
    echo "Пожалуйста, установите недостающие пакеты и попробуйте снова."
    exit 2
fi

# Очистка предыдущих результатов
rm -rf $BUILD_DIR/*

# Создать каталог для сборки
mkdir -p $BUILD_DIR
cd $BUILD_DIR

# Конфигурация
cmake .. -DCMAKE_TOOLCHAIN_FILE=arm-none-eabi-toolchain.cmake -DCMAKE_BUILD_TYPE=DebugQuiet
# cmake .. -DCMAKE_TOOLCHAIN_FILE=arm-none-eabi-toolchain.cmake 

# Сборка
make

# Проверка успешности сборки
if [ $? -eq 0 ]; then
    echo "Сборка прошла успешно!"
    echo "Созданные файлы:"
    ls -la *.elf *.hex *.bin 2>/dev/null || echo "Выходные файлы не найдены"
else
    echo "Сборка не удалась! Проверьте сообщения об ошибках выше."
    exit 1
fi

cd ..
