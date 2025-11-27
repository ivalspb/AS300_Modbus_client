cmake_minimum_required(VERSION 3.16)

# GUI components
set(GUI_WIDGETS_SOURCES
    gui/widgets/ChartWidget.h
    gui/widgets/ChartWidget.cpp
    gui/widgets/ConnectionWidget.h
    gui/widgets/ConnectionWidget.cpp
    gui/widgets/MonitorWidget.h
    gui/widgets/MonitorWidget.cpp
    gui/widgets/DualIndicatorWidget.h
    gui/widgets/DualIndicatorWidget.cpp
    gui/widgets/SpeedometerWidget.h
    gui/widgets/SpeedometerWidget.cpp
)

set(GUI_FACTORIES_SOURCES
    gui/factories/WidgetFactory.h
    gui/factories/WidgetFactory.cpp
)

set(GUI_MAINWINDOW_SOURCES
    gui/mainwindow/MainWindow.h
    gui/mainwindow/MainWindow.cpp
)

set(GUI_CONNECTION_SOURCES
    gui/connection/ConnectionViewController.h
    gui/connection/ConnectionViewController.cpp
)

set(GUI_DATABASE_SOURCES
    gui/database/DatabaseViewController.h
    gui/database/DatabaseViewController.cpp
)

set(GUI_MONITORING_SOURCES
    gui/monitoring/MonitoringViewController.h
    gui/monitoring/MonitoringViewController.cpp
)

set(GUI_MODE_SELECTION_SOURCES
    gui/mode_selection/ModeSelectionViewController.h
    gui/mode_selection/ModeSelectionViewController.cpp
)

# Все GUI исходники
set(GUI_SOURCES
    ${GUI_WIDGETS_SOURCES}
    ${GUI_FACTORIES_SOURCES}
    ${GUI_MAINWINDOW_SOURCES}
    ${GUI_CONNECTION_SOURCES}
    ${GUI_DATABASE_SOURCES}
    ${GUI_MONITORING_SOURCES}
    ${GUI_MODE_SELECTION_SOURCES}
)

# Добавляем к основной библиотеке (если она определена)
if(TARGET ModbusCore)
    target_sources(ModbusCore PRIVATE ${GUI_SOURCES})
endif()

## Группируем в IDE
#source_group("Widgets" FILES ${GUI_WIDGETS_SOURCES})
#source_group("Factories" FILES ${GUI_FACTORIES_SOURCES})
#source_group("MainWindow" FILES ${GUI_MAINWINDOW_SOURCES})
#source_group("Connection" FILES ${GUI_CONNECTION_SOURCES})
#source_group("Database" FILES ${GUI_DATABASE_SOURCES})
#source_group("Monitoring" FILES ${GUI_MONITORING_SOURCES})
#source_group("GUI\\\\ModeSelection" FILES ${GUI_MODE_SELECTION_SOURCES})

