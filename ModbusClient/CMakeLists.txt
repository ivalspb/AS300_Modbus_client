cmake_minimum_required(VERSION 3.16)
project(ModbusClient VERSION 1.0.0 LANGUAGES CXX)

# Общие настройки для всего проекта
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Автоматизация для Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Политики CMake
if(POLICY CMP0071)
    cmake_policy(SET CMP0071 NEW)
endif()

# Опции конфигурации
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)

# Поиск Qt
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Charts Network SerialBus Sql)

# Поиск SQLite
find_package(PkgConfig REQUIRED)
#pkg_check_modules(SQLite3 REQUIRED sqlite3)

# Основное приложение
add_subdirectory(src)

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Примеры (опционально)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Установка
install(TARGETS ModbusClient
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

# Пакетирование (опционально)
include(CPack)
