#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Использование: $0 <структурный_файл>"
    exit 1
fi

INPUT_FILE="$1"

if [ ! -f "$INPUT_FILE" ]; then
    echo "Файл '$INPUT_FILE' не существует"
    exit 1
fi

# Функция для определения уровня вложенности и очистки имени
parse_line() {
    local line="$1"

    # Определяем уровень вложенности по количеству символов дерева в начале
    local prefix=$(echo "$line" | grep -o '^[├└│─┬┌ ]*')
    local level=$(($(echo -n "$prefix" | wc -c) / 4))

    # Очищаем имя от символов дерева
    local clean_name=$(echo "$line" | sed -E 's/^[├└│─┬┌ ]*//')

    echo "$level:$clean_name"
}

# Создаем структуру
current_level=0
declare -a path_stack=()

while IFS= read -r line; do
    # Пропускаем пустые строки
    [[ -z "$line" ]] && continue

    # Парсим строку
    IFS=':' read -r level name <<< "$(parse_line "$line")"

    # Пропускаем, если после очистки ничего не осталось
    [[ -z "$name" ]] && continue

    # Определяем тип элемента
    if [[ "$name" =~ /$ ]]; then
        # Это директория
        dir_name="${name%/}"

        # Корректируем стек пути в зависимости от уровня
        while [ $current_level -ge $level ]; do
            unset path_stack[$current_level]
            ((current_level--))
        done

        # Добавляем текущую директорию в стек
        path_stack[$level]="$dir_name"
        current_level=$level

        # Формируем полный путь
        full_path=""
        for ((i=0; i<=level; i++)); do
            if [ -n "${path_stack[$i]}" ]; then
                if [ -z "$full_path" ]; then
                    full_path="${path_stack[$i]}"
                else
                    full_path="$full_path/${path_stack[$i]}"
                fi
            fi
        done

        mkdir -p "$full_path"
        echo "Создана директория: $full_path"
    else
        # Это файл
        # Корректируем стек пути для файла
        temp_level=$current_level
        declare -a temp_stack=("${path_stack[@]}")

        while [ $temp_level -ge $level ]; do
            unset temp_stack[$temp_level]
            ((temp_level--))
        done

        # Формируем полный путь к файлу
        full_path=""
        for ((i=0; i<level; i++)); do
            if [ -n "${temp_stack[$i]}" ]; then
                if [ -z "$full_path" ]; then
                    full_path="${temp_stack[$i]}"
                else
                    full_path="$full_path/${temp_stack[$i]}"
                fi
            fi
        done

        if [ -n "$full_path" ]; then
            file_path="$full_path/$name"
        else
            file_path="$name"
        fi

        # Создаем родительские директории и файл
        mkdir -p "$(dirname "$file_path")"
        touch "$file_path"
        echo "Создан файл: $file_path"
    fi
done < "$INPUT_FILE"

echo "Структура проекта создана успешно!"
