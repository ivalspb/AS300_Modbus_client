cmake_minimum_required(VERSION 3.16)

# Core interfaces
set(CORE_INTERFACES
    core/interfaces/IModbusClient.h
    core/interfaces/IRequestQueue.h
    core/interfaces/IIndicator.h
    core/interfaces/IAddressMapper.h
    core/interfaces/IPollingConfigurator.h
)

# Core modbus implementation
set(CORE_MODBUS_SOURCES
    core/modbus/ModbusRequestQueue.h
    core/modbus/ModbusRequestQueue.cpp
    core/modbus/ModbusRequestHandler.h
    core/modbus/ModbusRequestHandler.cpp
    core/modbus/DeltaModbusClient.h
    core/modbus/DeltaModbusClient.cpp
    core/modbus/CustomModbusClient.h
    core/modbus/CustomModbusClient.cpp
    core/modbus/factoies/PollingConfiguratorFactory.cpp
    core/modbus/factoies/PollingConfiguratorFactory.h
)

# Core mapping
set(CORE_MAPPING_SOURCES
    core/mapping/DeltaAddressMapper.h
    core/mapping/DeltaAddressMapper.cpp
    core/mapping/DeltaAddressMap.h
    core/mapping/DeltaController.h
)

# Core connection
set(CORE_CONNECTION_SOURCES
    core/connection/ConnectionManager.h
    core/connection/ConnectionManager.cpp
)

# Data layer
set(DATA_LAYER_SOURCES
    data/interfaces/IDataRepository.h
    data/DataPoint.h
    data/DataRepository.h
    data/DataRepository.cpp
)

# Database layer
set(DATABASE_LAYER_SOURCES
    data/database/IDatabaseRepository.h
    data/database/SqliteDatabaseRepository.h
    data/database/SqliteDatabaseRepository.cpp
    data/database/TestSession.h
    data/database/TestSessionDao.h
    data/database/TestSessionDao.cpp
    data/database/DataPointDao.h
    data/database/DataPointDao.cpp
)

# Database async manager
set(DATABASE_ASYNC_SOURCES
    data/database/DatabaseAsyncManager.h
    data/database/DatabaseAsyncManager.cpp
)

# Database export
set(DATABASE_EXPORT_SOURCES
    data/database/DatabaseExportService.h
    data/database/DatabaseExportService.cpp
)


# Monitoring
set(MONITORING_SOURCES
    monitoring/DataMonitor.h
    monitoring/DataMonitor.cpp
    monitoring/DiscreteInputMonitor.h
    monitoring/DiscreteInputMonitor.cpp
    monitoring/AnalogValueMonitor.h
    monitoring/AnalogValueMonitor.cpp
)

# Control State Machine
set(CONTROL_STATE_SOURCES
    control/ControlStateMachine.h
    control/ControlStateMachine.cpp
    control/ControlUIController.h
    control/ControlUIController.cpp
)

# Control
set(CONTROL_SOURCES
    control/ModeController.h
    control/ModeController.cpp
    control/ParameterController.h
    control/ParameterController.cpp
    ${CONTROL_STATE_SOURCES}
)

# GUI
set(GUI_SOURCES
    gui/MainWindow.h
    gui/MainWindow.cpp
    gui/widgets/ChartWidget.h
    gui/widgets/ChartWidget.cpp
    gui/widgets/SpeedometerWidget.h
    gui/widgets/SpeedometerWidget.cpp
    gui/widgets/ConnectionWidget.h
    gui/widgets/ConnectionWidget.cpp
    gui/widgets/MonitorWidget.h
    gui/widgets/MonitorWidget.cpp
    gui/widgets/DualIndicatorWidget.h
    gui/widgets/DualIndicatorWidget.cpp
    gui/factories/WidgetFactory.h
    gui/factories/WidgetFactory.cpp
)

# Export
set(EXPORT_SOURCES
    export/interfaces/IExportStrategy.h
    export/PngExportStrategy.h
    export/PngExportStrategy.cpp
)

# ВСЕ исходники кроме main.cpp для библиотеки
set(LIB_SOURCES
    ${CORE_INTERFACES}
    ${CORE_MODBUS_SOURCES}
    ${CORE_MAPPING_SOURCES}
    ${CORE_CONNECTION_SOURCES}
    ${DATA_LAYER_SOURCES}
    ${DATABASE_LAYER_SOURCES}
    ${DATABASE_ASYNC_SOURCES}
    ${DATABASE_EXPORT_SOURCES}
    ${MONITORING_SOURCES}
    ${CONTROL_SOURCES}
    ${GUI_SOURCES}
    ${EXPORT_SOURCES}
)

# 1. Создаем СТАТИЧЕСКУЮ библиотеку со всей логикой
add_library(ModbusCore STATIC ${LIB_SOURCES})

# Группируем файлы в IDE
source_group("Core Interfaces" FILES ${CORE_INTERFACES})
source_group("Core Modbus" FILES ${CORE_MODBUS_SOURCES})
source_group("Core Mapping" FILES ${CORE_MAPPING_SOURCES})
source_group("Core Connection" FILES ${CORE_CONNECTION_SOURCES})
source_group("Data Layer" FILES ${DATA_LAYER_SOURCES})
source_group("Monitoring" FILES ${MONITORING_SOURCES})
source_group("Control" FILES ${CONTROL_SOURCES})
source_group("GUI" FILES ${GUI_SOURCES})
source_group("Export" FILES ${EXPORT_SOURCES})
source_group("Database Layer" FILES ${DATABASE_LAYER_SOURCES})
source_group("Database Async" FILES ${DATABASE_ASYNC_SOURCES})
source_group("Database Export" FILES ${DATABASE_EXPORT_SOURCES})
source_group("Control State Machine" FILES ${CONTROL_STATE_SOURCES})

find_package(PkgConfig REQUIRED)
#pkg_check_modules(SQLite3 REQUIRED sqlite3)

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
#    ${Qt5Sql_INCLUDE_DIRS}
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

# 2. Создаем исполняемое приложение, которое использует библиотеку
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

install(TARGETS ModbusCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
