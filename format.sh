#!/bin/bash

FILE=$1

# Проверка существования файла
if [ ! -f "$FILE" ]; then
    echo "Ошибка: файл '$FILE' не существует"
    exit 1
fi

# Проверка расширения файла
if [[ ! "$FILE" =~ \.(c|h)$ ]]; then
    echo "Ошибка: файл должен иметь расширение .c или .h"
    exit 1
fi

# Проверка, установлен ли clang-format
if ! command -v clang-format > /dev/null 2>&1
then
    echo "Ошибка: clang-format не установлен"
    exit 1
fi

# Проверка наличия файла .clang-format
if [ ! -f .clang-format ]; then
    echo "Предупреждение: файл .clang-format не найден, будет использован стиль по умолчанию"
fi

# Форматирование указанного файла
echo "Запуск clang-format для файла $FILE..."
clang-format -style=file -i "$FILE"

# Замена пробелов в фигурных скобках ПОСЛЕ форматирования
# Удаляем пробелы ТОЛЬКО в {числах}, например { 0 } -> {0}
echo "Удаление пробелов в числовых скобках..."
if [[ "$OSTYPE" == "darwin"* ]]; then
  sed -i '' -E 's/\{[[:space:]]*([0-9]+)[[:space:]]*\}/\{\1\}/g' "$FILE"
else
  sed -i -E 's/\{\s*([0-9]+)\s*\}/{\1}/g' "$FILE"
fi

echo "Форматирование завершено для файла $FILE"
