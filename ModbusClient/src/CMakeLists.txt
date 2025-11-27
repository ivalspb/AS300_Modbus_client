cmake_minimum_required(VERSION 3.16)

# Core interfaces and implementation
set(CORE_SOURCES
    core/interfaces/IModbusClient.h
    core/interfaces/IRequestQueue.h
    core/interfaces/IIndicator.h
    core/interfaces/IAddressMapper.h
    core/modbus/ModbusRequestQueue.h
    core/modbus/ModbusRequestQueue.cpp
    core/modbus/ModbusRequestHandler.h
    core/modbus/ModbusRequestHandler.cpp
    core/modbus/DeltaModbusClient.h
    core/modbus/DeltaModbusClient.cpp
    core/modbus/CustomModbusClient.h
    core/modbus/CustomModbusClient.cpp
    core/mapping/DeltaAddressMapper.h
    core/mapping/DeltaAddressMapper.cpp
    core/mapping/DeltaAddressMap.h
    core/mapping/DeltaController.h
    core/connection/ConnectionManager.h
    core/connection/ConnectionManager.cpp
)

# Data layer
set(DATA_SOURCES
    data/interfaces/IDataRepository.h
    data/DataPoint.h
    data/DataRepository.h
    data/DataRepository.cpp
    data/database/IDatabaseRepository.h
    data/database/SqliteDatabaseRepository.h
    data/database/SqliteDatabaseRepository.cpp
    data/database/TestSession.h
    data/database/TestSessionDao.h
    data/database/TestSessionDao.cpp
    data/database/DataPointDao.h
    data/database/DataPointDao.cpp
    data/database/DatabaseAsyncManager.h
    data/database/DatabaseAsyncManager.cpp
    data/database/DatabaseExportService.h
    data/database/DatabaseExportService.cpp
)

# Monitoring and control
set(MONITORING_SOURCES
    monitoring/DataMonitor.h
    monitoring/DataMonitor.cpp
    monitoring/DiscreteInputMonitor.h
    monitoring/DiscreteInputMonitor.cpp
    monitoring/AnalogValueMonitor.h
    monitoring/AnalogValueMonitor.cpp
)

set(CONTROL_SOURCES
    control/ModeController.h
    control/ModeController.cpp
    control/ParameterController.h
    control/ParameterController.cpp
    control/ControlStateMachine.h
    control/ControlStateMachine.cpp
    control/ControlUIController.h
    control/ControlUIController.cpp
)

# Export
set(EXPORT_SOURCES
    export/interfaces/IExportStrategy.h
    export/PngExportStrategy.h
    export/PngExportStrategy.cpp
)

# GUI components (включаем все GUI файлы)
include(gui/CMakeLists.txt)

# Все исходники для библиотеки
set(LIB_SOURCES
    ${CORE_SOURCES}
    ${DATA_SOURCES}
    ${MONITORING_SOURCES}
    ${CONTROL_SOURCES}
    ${EXPORT_SOURCES}
    ${GUI_SOURCES}
)

# Создаем СТАТИЧЕСКУЮ библиотеку
add_library(ModbusCore STATIC ${LIB_SOURCES})

## Группируем файлы в IDE
#source_group("Core" FILES ${CORE_SOURCES})
#source_group("Data" FILES ${DATA_SOURCES})
#source_group("Monitoring" FILES ${MONITORING_SOURCES})
#source_group("Control" FILES ${CONTROL_SOURCES})
#source_group("Export" FILES ${EXPORT_SOURCES})

# Подключаем зависимости к библиотеке
target_link_libraries(ModbusCore
    Qt5::Core
    Qt5::Widgets
    Qt5::Charts
    Qt5::Network
    Qt5::SerialBus
    Qt5::Sql
    ${SQLite3_LIBRARIES}
)

# Директории include для библиотеки
target_include_directories(ModbusCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

# Определения компилятора
target_compile_definitions(ModbusCore PRIVATE
    QT_DEPRECATED_WARNINGS
)

# Свойства цели библиотеки
set_target_properties(ModbusCore PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Создаем исполняемое приложение
add_executable(ModbusClient main.cpp)

# Приложение линкуется с нашей библиотекой
target_link_libraries(ModbusClient
    ModbusCore
    Qt5::Core
    Qt5::Widgets
    Qt5::Charts
    Qt5::Network
    Qt5::SerialBus
    Qt5::Sql
    ${SQLite3_LIBRARIES}
)

# Установка
install(TARGETS ModbusClient
    RUNTIME DESTINATION bin
)
