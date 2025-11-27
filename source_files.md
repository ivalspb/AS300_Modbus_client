# Список C++ файлов и их содержимое

## Файл: structure.txt

```cpp
ModbusClient/
├── CMakeLists.txt
├── src/
│ ├── main.cpp
│ ├── core/
│ │ ├── interfaces/
│ │ │ ├── IModbusClient.h
│ │ │ ├── IRequestQueue.h
│ │ │ └── IAddressMapper.h
│ │ ├── modbus/
│ │ │ ├── ModbusRequestQueue.h
│ │ │ ├── ModbusRequestQueue.cpp
│ │ │ ├── ModbusRequestHandler.h
│ │ │ ├── ModbusRequestHandler.cpp
│ │ │ ├── DeltaModbusClient.h
│ │ │ ├── DeltaModbusClient.cpp
│ │ │ ├── CustomModbusClient.h
│ │ │ └── CustomModbusClient.cpp
│ │ ├── mapping/
│ │ │ ├── DeltaAddressMapper.h
│ │ │ ├── DeltaAddressMapper.cpp
│ │ │ ├── DeltaAddressMap.h
│ │ │ └── DeltaController.h
│ │ └── connection/
│ │ ├── ConnectionManager.h
│ │ ├── ConnectionManager.cpp
│ │ ├── ConnectionTester.h
│ │ └── ConnectionTester.cpp
│ ├── data/
│ │ ├── interfaces/
│ │ │ └── IDataRepository.h
│ │ ├── DataRepository.h
│ │ ├── DataRepository.cpp
│ │ └── DataPoint.h
│ ├── monitoring/
│ │ ├── DataMonitor.h
│ │ ├── DataMonitor.cpp
│ │ ├── DiscreteInputMonitor.h
│ │ ├── DiscreteInputMonitor.cpp
│ │ ├── AnalogValueMonitor.h
│ │ └── AnalogValueMonitor.cpp
│ ├── control/
│ │ ├── ModeController.h
│ │ ├── ModeController.cpp
│ │ ├── ParameterController.h
│ │ └── ParameterController.cpp
│ ├── gui/
│ │ ├── MainWindow.h
│ │ ├── MainWindow.cpp
│ │ ├── widgets/
│ │ │ ├── ChartWidget.h
│ │ │ ├── ChartWidget.cpp
│ │ │ ├── ConnectionWidget.h
│ │ │ ├── ConnectionWidget.cpp
│ │ │ ├── MonitorWidget.h
│ │ │ └── MonitorWidget.cpp
│ │ └── factories/
│ │ ├── WidgetFactory.h
│ │ └── WidgetFactory.cpp
│ └── export/
│ ├── interfaces/
│ │ └── IExportStrategy.h
│ ├── PngExportStrategy.h
│ └── PngExportStrategy.cpp
└── tests/

```

---


## Файл: ModbusClient/structure.txt

```cpp
tests/
├── CMakeLists.txt
├── test_main.cpp
├── unit/
│   ├── core/
│   │   ├── test_modbus_request_queue.cpp
│   │   ├── test_address_mapper.cpp
│   │   └── test_connection_manager.cpp
│   ├── data/
│   │   └── test_data_repository.cpp
│   ├── monitoring/
│   │   └── test_monitors.cpp
│   └── control/
│       └── test_mode_controller.cpp
├── integration/
│   └── test_modbus_integration.cpp
└── mocks/
    ├── MockModbusClient.h
    ├── MockModbusClient.cpp
    ├── MockDataRepository.h
    └── MockDataRepository.cpp
```

---


## Файл: ModbusClient/src/monitoring/DataMonitor.h

```cpp
#pragma once
#include <QObject>
#include <QTimer>

class IModbusClient;
class IDataRepository;
class DiscreteInputMonitor;
class AnalogValueMonitor;

class DataMonitor : public QObject {
    Q_OBJECT
public:
    explicit DataMonitor(IModbusClient* client,
                         IDataRepository* repository,
                         QObject* parent = nullptr);
    ~DataMonitor() override;

    DiscreteInputMonitor* discreteMonitor() const { return m_discreteMonitor; }
    AnalogValueMonitor* analogMonitor() const { return m_analogMonitor; }

    void startMonitoring();
    void stopMonitoring();
    void setUpdateInterval(int ms);

signals:
    void dataUpdated(const QString& parameter, double value);
    void logMessage(const QString& message);

private slots:
    void updateData();
    void onDiscreteStatusChanged(int input, bool value);
    void onAnalogValueChanged(int analog, double value);
    void onCommandChanged(int output, bool value);

private:
    IModbusClient* m_client;
    IDataRepository* m_repository;
    DiscreteInputMonitor* m_discreteMonitor;
    AnalogValueMonitor* m_analogMonitor;
    QTimer* m_updateTimer;
};


```

---


## Файл: ModbusClient/src/monitoring/AnalogValueMonitor.h

```cpp
#pragma once
#include <QObject>
#include <QModbusDataUnit>
#include "core/interfaces/IIndicator.h"

class IModbusClient;
class IAddressMapper;

class AnalogValueMonitor : public QObject {
    Q_OBJECT
public:
    explicit AnalogValueMonitor(IModbusClient* client,
                                IAddressMapper* mapper,
                                QObject* parent = nullptr);

    void setIndicators(IDualIndicator* adIndicator,
                       IDualIndicator* tkIndicator,
                       IDualIndicator* stIndicator);

    void startMonitoring();
    void stopMonitoring();

signals:
    void valueChanged(int analog, double value);

private slots:
    void onRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);
    void onRegistersRead(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values);

private:
    double convertValue(int analog, quint16 rawValue);
    double convertDWORDValue(int analog, const QVector<quint16>& values);

    IModbusClient* m_client;
    IAddressMapper* m_mapper;
    IDualIndicator* m_adIndicator;
    IDualIndicator* m_tkIndicator;
    IDualIndicator* m_stIndicator;
};

```

---


## Файл: ModbusClient/src/monitoring/DiscreteInputMonitor.h

```cpp
#pragma once
#include <QObject>
#include <QLabel>
#include <QVector>
#include <QMap>
#include <QModbusDataUnit>

class IModbusClient;
class IAddressMapper;

class DiscreteInputMonitor : public QObject {
    Q_OBJECT
public:
    explicit DiscreteInputMonitor(IModbusClient* client,
                                  IAddressMapper* mapper,
                                  QObject* parent = nullptr);

    void setDiscreteLabels(const QVector<QLabel*>& labels);
    void setCommandLabels(const QVector<QLabel*>& labels);
    void startMonitoring();
    void stopMonitoring();

signals:
    void statusChanged(int input, bool value);
    void commandChanged(int output, bool value);

private slots:
    void onRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);

private:
    void updateIndicator(QLabel* label, bool value);
    void updateDiscreteIndicator(int input, bool value);
    void updateCommandIndicator(int output, bool value);

    IModbusClient* m_client;
    IAddressMapper* m_mapper;
    QVector<QLabel*> m_discreteLabels;
    QVector<QLabel*> m_commandLabels;
    QMap<quint16, int> m_addressToDiscreteMap;
    QMap<quint16, int> m_addressToCommandMap;
};

```

---


## Файл: ModbusClient/src/monitoring/AnalogValueMonitor.cpp

```cpp
#include "AnalogValueMonitor.h"
#include "core/interfaces/IModbusClient.h"
#include "core/interfaces/IAddressMapper.h"
#include "core/mapping/DeltaController.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QtMath>
#include <QDebug>

AnalogValueMonitor::AnalogValueMonitor(IModbusClient* client,
                                       IAddressMapper* mapper,
                                       QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_mapper(mapper)
    , m_adIndicator(nullptr)
    , m_tkIndicator(nullptr)
    , m_stIndicator(nullptr)
{
    connect(m_client, SIGNAL(registerReadCompleted(QModbusDataUnit::RegisterType,quint16,quint16)),
            this, SLOT(onRegisterRead(QModbusDataUnit::RegisterType,quint16,quint16)));
    connect(m_client, SIGNAL(registersReadCompleted(QModbusDataUnit::RegisterType,quint16,QVector<quint16>)),
            this, SLOT(onRegistersRead(QModbusDataUnit::RegisterType,quint16,QVector<quint16>)));
}

void AnalogValueMonitor::setIndicators(IDualIndicator* adIndicator,
                                       IDualIndicator* tkIndicator,
                                       IDualIndicator* stIndicator) {
    m_adIndicator = adIndicator;
    m_tkIndicator = tkIndicator;
    m_stIndicator = stIndicator;

    // Настраиваем диапазоны
    if (m_adIndicator) {
        m_adIndicator->setRange(0, 110); // Проценты
        m_adIndicator->setSecondaryRange(0, 4996); // RPM (110% от 4542)
    }
    if (m_tkIndicator) {
        m_tkIndicator->setRange(0, 110);
        m_tkIndicator->setSecondaryRange(0, 64750);
    }
    if (m_stIndicator) {
        m_stIndicator->setRange(0, 110);
        m_stIndicator->setSecondaryRange(0, 71500);
    }
}

void AnalogValueMonitor::startMonitoring() {
    // Опрос теперь управляется централизованно в DeltaModbusClient
    // Этот метод может быть пустым или удален
}

void AnalogValueMonitor::stopMonitoring() {
    if (m_adIndicator) m_adIndicator->setValue(0);
    if (m_tkIndicator) m_tkIndicator->setValue(0);
    if (m_stIndicator) m_stIndicator->setValue(0);
}

void AnalogValueMonitor::onRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) {
    if (address == m_mapper->getAnalogAddress(DeltaController::AD_RPM)) {
        double rpm = convertValue(DeltaController::AD_RPM, value);
        double percent = (rpm / 4542.0) * 100.0;

        if (m_adIndicator) {
            m_adIndicator->setValue(percent);
            m_adIndicator->setSecondaryValue(rpm);
        }
        emit valueChanged(DeltaController::AD_RPM, rpm);
    }
}

void AnalogValueMonitor::onRegistersRead(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values) {
    if (address == m_mapper->getAnalogAddress(DeltaController::TK_PERCENT)) {
        double percent = convertDWORDValue(DeltaController::TK_PERCENT, values);
        if (m_tkIndicator) {
            m_tkIndicator->setValue(percent);
        }
        emit valueChanged(DeltaController::TK_PERCENT, percent);
    }
    else if (address == m_mapper->getAnalogAddress(DeltaController::TK_RPM)) {
        double rpm = convertDWORDValue(DeltaController::TK_RPM, values);
        if (m_tkIndicator) {
            m_tkIndicator->setSecondaryValue(rpm);
        }
        emit valueChanged(DeltaController::TK_RPM, rpm);
    }
    else if (address == m_mapper->getAnalogAddress(DeltaController::ST_PERCENT)) {
        double percent = convertDWORDValue(DeltaController::ST_PERCENT, values);

        if (m_stIndicator) {
            m_stIndicator->setValue(percent);
        }
        emit valueChanged(DeltaController::ST_PERCENT, percent);
    }
    else if (address == m_mapper->getAnalogAddress(DeltaController::ST_RPM)) {
        double rpm = convertDWORDValue(DeltaController::ST_RPM, values);
        if (m_stIndicator) {
            m_stIndicator->setSecondaryValue(rpm);
        }
        emit valueChanged(DeltaController::ST_RPM, rpm);
    }
}

double AnalogValueMonitor::convertValue(int analog, quint16 rawValue) {
    switch (analog) {
    case DeltaController::AD_RPM:
        return DeltaAS332T::scaleRPM(rawValue);
    default:
        return static_cast<double>(rawValue);
    }
}

double AnalogValueMonitor::convertDWORDValue(int analog, const QVector<quint16>& values) {
    quint32 dwordValue = DeltaAS332T::readDWORD(values, 0);

    switch (analog) {
    case DeltaController::TK_RPM:
    case DeltaController::ST_RPM:
        return DeltaAS332T::scaleRPM(dwordValue);
    case DeltaController::TK_PERCENT:
    case DeltaController::ST_PERCENT:
        return DeltaAS332T::scalePercent(dwordValue);
    default:
        return dwordValue;
    }
}

```

---


## Файл: ModbusClient/src/monitoring/DataMonitor.cpp

```cpp
#include "DataMonitor.h"
#include "DiscreteInputMonitor.h"
#include "AnalogValueMonitor.h"
#include "core/interfaces/IModbusClient.h"
#include "data/interfaces/IDataRepository.h"
#include "core/mapping/DeltaAddressMapper.h"
#include <QDebug>

DataMonitor::DataMonitor(IModbusClient* client,
                         IDataRepository* repository,
                         QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_repository(repository)
    , m_updateTimer(new QTimer(this))
{
    // Create address mapper
    IAddressMapper* mapper = new DeltaAddressMapper();

    // Create sub-monitors
    m_discreteMonitor = new DiscreteInputMonitor(client, mapper, this);
    m_analogMonitor = new AnalogValueMonitor(client, mapper, this);

    // Connect signals
    connect(m_discreteMonitor, SIGNAL(statusChanged(int,bool)), this, SLOT(onDiscreteStatusChanged(int,bool)));
    connect(m_analogMonitor, SIGNAL(valueChanged(int,double)), this, SLOT(onAnalogValueChanged(int,double)));

    connect(m_updateTimer, &QTimer::timeout, this, &DataMonitor::updateData);
    connect(m_discreteMonitor, &DiscreteInputMonitor::commandChanged,
            this, &DataMonitor::onCommandChanged);

    m_updateTimer->setInterval(1000);
}

DataMonitor::~DataMonitor() {
    if (m_updateTimer) {
        m_updateTimer->stop();
        disconnect(m_updateTimer, nullptr, this, nullptr); // Отключаем все соединения
    }
    if (m_discreteMonitor) {
        disconnect(m_discreteMonitor, nullptr, this, nullptr);
    }
    if (m_analogMonitor) {
        disconnect(m_analogMonitor, nullptr, this, nullptr);
    }
    if (m_client) {
        disconnect(m_client, nullptr, this, nullptr);
    }
    m_updateTimer->deleteLater(); // Безопасное удаление таймера
}

void DataMonitor::startMonitoring() {
    if (!m_client->isConnected()) {
        emit logMessage("Not connected to device, monitoring not started");
        return;
    }

    // m_updateTimer->start();
    // m_discreteMonitor->startMonitoring();
    // m_analogMonitor->startMonitoring();

    emit logMessage("Data monitoring started with optimized polling frequencies");
}

void DataMonitor::stopMonitoring() {
    if (m_updateTimer) {
        m_updateTimer->stop();
    } else {
    }
    if (m_discreteMonitor) {
        m_discreteMonitor->stopMonitoring();
    } else {
    }
    if (m_analogMonitor) {
        m_analogMonitor->stopMonitoring();
    } else {
    }
    if (m_client) {
        disconnect(m_client, nullptr, this, nullptr);
    }
}

void DataMonitor::setUpdateInterval(int ms) {
    m_updateTimer->setInterval(qMax(100, ms));
}

void DataMonitor::updateData() {
    // Этот метод теперь может быть пустым или использоваться
    // для дополнительной логики, не связанной с опросом
    if (!m_client->isConnected()) {
        // Останавливаем мониторинг при потере соединения
        stopMonitoring();
        return;
    }
}

void DataMonitor::onDiscreteStatusChanged(int input, bool value) {
    QString paramName = QString("DiscreteInput_%1").arg(input);
    double numValue = value ? 1.0 : 0.0;

    m_repository->addDataPoint(paramName, numValue);
    emit dataUpdated(paramName, numValue);
}

void DataMonitor::onAnalogValueChanged(int analog, double value) {
    QString paramName;
    switch (analog) {
    case DeltaController::AD_RPM:
        paramName = "AD_RPM";
        break;
    case DeltaController::TK_RPM:
        paramName = "TK_RPM";
        break;
    case DeltaController::TK_PERCENT:
        paramName = "TK_PERCENT";
        break;
    case DeltaController::ST_RPM:
        paramName = "ST_RPM";
        break;
    case DeltaController::ST_PERCENT:
        paramName = "ST_PERCENT";
        break;
    default:
        paramName = QString("Analog_%1").arg(analog);
        break;
    }

    m_repository->addDataPoint(paramName, value);
    emit dataUpdated(paramName, value);
}

void DataMonitor::onCommandChanged(int output, bool value) {
    QString paramName = QString("Command_%1").arg(output);
    double numValue = value ? 1.0 : 0.0;

    m_repository->addDataPoint(paramName, numValue);
    emit dataUpdated(paramName, numValue);
}

```

---


## Файл: ModbusClient/src/monitoring/DiscreteInputMonitor.cpp

```cpp
#include "DiscreteInputMonitor.h"
#include "core/interfaces/IModbusClient.h"
#include "core/interfaces/IAddressMapper.h"
#include "core/mapping/DeltaController.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QModbusDataUnit>

DiscreteInputMonitor::DiscreteInputMonitor(IModbusClient* client,
                                           IAddressMapper* mapper,
                                           QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_mapper(mapper)
{
    // Build reverse mapping for discrete inputs
    for (int i = 0; i < 12; ++i) {
        quint16 addr = m_mapper->getDiscreteInputAddress(i);
        m_addressToDiscreteMap[addr] = i;
    }

    // Build reverse mapping for command outputs
    for (int i = 0; i < 6; ++i) {
        quint16 addr = m_mapper->getCommandOutputAddress(i);
        m_addressToCommandMap[addr] = i;
    }

    connect(m_client, SIGNAL(registerReadCompleted(QModbusDataUnit::RegisterType,quint16,quint16)),
            this, SLOT(onRegisterRead(QModbusDataUnit::RegisterType,quint16,quint16)));
}

void DiscreteInputMonitor::setDiscreteLabels(const QVector<QLabel*>& labels) {
    m_discreteLabels = labels;
}

void DiscreteInputMonitor::setCommandLabels(const QVector<QLabel*>& labels) {
    m_commandLabels = labels;
}

void DiscreteInputMonitor::startMonitoring() {
    // Опрос теперь управляется централизованно в DeltaModbusClient
    // Этот метод может быть пустым или удален
}

void DiscreteInputMonitor::stopMonitoring() {
    // Reset all discrete labels
    for (QLabel* label : m_discreteLabels) {
        if (label) {
            updateIndicator(label, false);
        }
    }

    // Reset all command labels
    for (QLabel* label : m_commandLabels) {
        if (label) {
            updateIndicator(label, false);
        }
    }
}

void DiscreteInputMonitor::onRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) {
    // Check if this is a discrete input
    if (m_addressToDiscreteMap.contains(address)) {
        int input = m_addressToDiscreteMap[address];
        bool state = (value > 0);
        updateDiscreteIndicator(input, state);
        emit statusChanged(input, state);
    }

    // Check if this is a command output
    else if (m_addressToCommandMap.contains(address)) {
        int output = m_addressToCommandMap[address];
        bool state = (value > 0);
        updateCommandIndicator(output, state);
        emit commandChanged(output, state);
    }
}

void DiscreteInputMonitor::updateIndicator(QLabel* label, bool value) {
    if (label) {
        if (value) {
            label->setStyleSheet("background-color: green; color: white; padding: 8px; border-radius: 4px;");
        } else {
            label->setStyleSheet("background-color: grey; color: white; padding: 8px; border-radius: 4px;");
        }
    }
}

void DiscreteInputMonitor::updateDiscreteIndicator(int input, bool value) {
    if (input >= 0 && input < m_discreteLabels.size() && m_discreteLabels[input]) {
        updateIndicator(m_discreteLabels[input], value);
    }
}

void DiscreteInputMonitor::updateCommandIndicator(int output, bool value) {
    if (output >= 0 && output < m_commandLabels.size() && m_commandLabels[output]) {
        updateIndicator(m_commandLabels[output], value);
    }
}

```

---


## Файл: ModbusClient/src/CMakeLists.txt

```cpp
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

```

---


## Файл: ModbusClient/src/control/ParameterController.h

```cpp
#pragma once
#include <QObject>
#include <QMap>

class IModbusClient;
class QComboBox;
class QLineEdit;
class QPushButton;

class ParameterController : public QObject {
    Q_OBJECT
public:
    explicit ParameterController(IModbusClient* client, QObject* parent = nullptr);

    void setParameterComboBox(QComboBox* comboBox);
    void setAddressEdit(QLineEdit* edit);
    void setAddButton(QPushButton* button);

    QMap<QString, quint16> parameterMap() const { return m_parameterMap; }

signals:
    void parameterAdded(const QString& name, quint16 address);
    void parameterSelected(const QString& name);
    void logMessage(const QString& message);

private slots:
    void onAddParameter();
    void onParameterSelected(int index);

private:
    IModbusClient* m_client;
    QComboBox* m_parameterCombo;
    QLineEdit* m_addressEdit;
    QPushButton* m_addButton;
    QMap<QString, quint16> m_parameterMap;
};


```

---


## Файл: ModbusClient/src/control/ModeController.h

```cpp
#pragma once
#include <QObject>
#include <QMap>
#include <QModbusDataUnit>

class IModbusClient;
class QPushButton;
class IDataRepository;

class ModeController : public QObject {
    Q_OBJECT
public:
    explicit ModeController(IModbusClient* client, QObject* parent = nullptr);

    void setConnectionState(bool connected);
    void setDataRepository(IDataRepository* repository) { m_dataRepository = repository; }
    QString currentMode() const { return m_currentMode; }

signals:
    void modeChanged(const QString& mode);
    void testStarted(const QString& mode);
    void testStopped();
    void logMessage(const QString& message);

public slots:
    void stopTest();  // Принудительная остановка теста
    void onMode1Clicked();
    void onMode2Clicked();
    void onMode3Clicked();
    void onMode4Clicked();
    void onMode5Clicked();
    void onStopClicked();
    void onRegisterWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success);
    void onSessionCreated(int sessionId);

private:
    void startTest(const QString& mode);
    void initializeModes();
    void writeModeRegister();

    IModbusClient* m_client;
    IDataRepository* m_dataRepository;
    QString m_currentMode;
    bool m_connected;
    bool m_testRunning;
};

```

---


## Файл: ModbusClient/src/control/ControlStateMachine.h

```cpp
#pragma once
#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QModbusDataUnit>


class IModbusClient;

class ControlStateMachine : public QObject {
    Q_OBJECT
public:
    enum State {
        STATE_READY_CHECK,    // Состояние 1: Проверка готовности
        STATE_START_INTERRUPT,// Состояние 2: Пуск/Прерывание
        STATE_STOP,           // Состояние 3: Стоп
        STATE_RESTART_EXIT    // Состояние 4: Повторение/Выход
    };
    Q_ENUM(State)

    explicit ControlStateMachine(IModbusClient* client, QObject* parent = nullptr);
    ~ControlStateMachine();

    State currentState() const { return m_currentState; }
    static void registerMetaTypes();

signals:
    void stateChanged(ControlStateMachine::State newState);
    void logMessage(const QString& message);
    void readyCheckRequested();
    void startRequested();
    void stopRequested();
    void restartRequested();
    void interruptRequested();
    void exitRequested();
    void modeSelectionRequested();
    void registerWriteVerified(const QString& registerName, quint16 value, bool success);
    void stopCurrentTest();

    void startChartRecording();
    void stopChartRecording();

    // Сигналы для переходов state machine
    void m11ReadySignal();
    void m12StartedSignal();
    void completionSignal();
    void startTriggered();
    void stopTriggered();
    void restartTriggered();
    void interruptTriggered();
    void exitTriggered();

public slots:
    void triggerReadyCheck();
    void triggerStart();
    void triggerStop();
    void triggerRestart();
    void triggerInterrupt();
    void triggerExit();

private slots:
    void transitionToReadyCheck();
    void transitionToStartInterrupt();
    void transitionToStop();
    void transitionToRestartExit();

    // НОВЫЙ слот для обработки статусных регистров
    void onStatusRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);

private:
    void setupStateMachine();
    void setupStatusMonitoring();
    void checkCompletionCondition();
    void resetStatusRegisters();

    void writeM1ReadyCheck();
    void writeM2Start();
    void writeM3Stop();
    void writeM4Restart();
    void writeM5Interrupt();
    void writeM6Exit();

    void writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address,
                                   quint16 value, const QString& registerName);
    void logRegisterOperation(const QString& operation, quint16 address,
                                 quint16 value, const QString& registerName, bool success = true);

    IModbusClient* m_client;
    State m_currentState;
    QStateMachine* m_stateMachine;
    QState* m_readyCheckState;
    QState* m_startInterruptState;
    QState* m_stopState;
    QState* m_restartExitState;

    // Статусные флаги для отслеживания регистров
    bool m_m11Ready;
    bool m_m12Started;
    bool m_m0Status;    // НОВЫЙ
    bool m_m14Status;   // НОВЫЙ
};

```

---


## Файл: ModbusClient/src/control/ParameterController.cpp

```cpp
#include "ParameterController.h"
#include "core/interfaces/IModbusClient.h"
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include <QDateTime>
#include <QModbusDataUnit>

ParameterController::ParameterController(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_parameterCombo(nullptr)
    , m_addressEdit(nullptr)
    , m_addButton(nullptr)
{}

void ParameterController::setParameterComboBox(QComboBox* comboBox) {
    m_parameterCombo = comboBox;
    if (m_parameterCombo) {
        connect(m_parameterCombo, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onParameterSelected(int)));
    }
}

void ParameterController::setAddressEdit(QLineEdit* edit) {
    m_addressEdit = edit;
    if (m_addressEdit) {
        m_addressEdit->setValidator(new QIntValidator(0, 65535, this));
    }
}

void ParameterController::setAddButton(QPushButton* button) {
    m_addButton = button;
    if (m_addButton) {
        connect(m_addButton, &QPushButton::clicked, this, &ParameterController::onAddParameter);
    }
}

void ParameterController::onAddParameter() {
    if (!m_parameterCombo || !m_addressEdit) return;

    QString paramName = m_parameterCombo->currentText();
    if (paramName.isEmpty()) {
        paramName = QString("Parameter_%1").arg(m_parameterCombo->count() + 1);
    }

    bool ok;
    quint16 address = m_addressEdit->text().toUShort(&ok);
    if (!ok) {
        emit logMessage("Error: Invalid register address");
        return;
    }

    // Add to polled registers
    if (m_client) {
        m_client->addPolledRegisterWithFrequency(paramName,
                                                 QModbusDataUnit::HoldingRegisters,
                                                 address,
                                                 1,
                                                 IModbusClient::LowFrequency);
    }

    m_parameterMap[paramName] = address;

    // Add to combo box if not already present
    if (m_parameterCombo->findText(paramName) == -1) {
        m_parameterCombo->addItem(paramName);
    }

    emit parameterAdded(paramName, address);
    emit logMessage(QString("[%1] Parameter added: %2 (address: 0x%3)")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                        .arg(paramName)
                        .arg(address, 4, 16, QChar('0')));
}

void ParameterController::onParameterSelected(int index) {
    if (index >= 0 && m_parameterCombo) {
        QString param = m_parameterCombo->currentText();
        emit parameterSelected(param);
    }
}

```

---


## Файл: ModbusClient/src/control/ModeController.cpp

```cpp
#include "ModeController.h"
#include "core/interfaces/IModbusClient.h"
#include "core/mapping/DeltaAddressMap.h"
#include "data/interfaces/IDataRepository.h"
#include <QPushButton>
#include <QDateTime>
#include <QModbusDataUnit>
#include <QTimer>
#include <QDebug>

ModeController::ModeController(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_dataRepository(nullptr)
    , m_connected(false)
    , m_testRunning(false)
{
    if (m_client) {
        connect(m_client, &IModbusClient::registerWriteVerified,
                this, &ModeController::onRegisterWriteVerified);
    }

    qDebug() << "ModeController: Created";
}

void ModeController::setConnectionState(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        qDebug() << "ModeController: Connection state changed to:" << connected;
        emit logMessage(connected ? "Устройство подключено" : "Устройство отключено");
    }
}

// Обработчики нажатий кнопок режимов
void ModeController::onMode1Clicked() {
    qDebug() << "ModeController: Mode 1 clicked";
    startTest("Расконсервация/Консервация");
}

void ModeController::onMode2Clicked() {
    qDebug() << "ModeController: Mode 2 clicked";
    startTest("Холодная прокрутка турбостартера");
}

void ModeController::onMode3Clicked() {
    qDebug() << "ModeController: Mode 3 clicked";
    startTest("Регулировка мощности, замер параметров");
}

void ModeController::onMode4Clicked() {
    qDebug() << "ModeController: Mode 4 clicked";
    startTest("Холодная прокрутка основного двигателя");
}

void ModeController::onMode5Clicked() {
    qDebug() << "ModeController: Mode 5 clicked";
    startTest("Имитация запуска основного двигателя");
}

void ModeController::startTest(const QString& mode) {
    if (!m_connected || m_testRunning) {
        qWarning() << "ModeController: Cannot start test - connected:" << m_connected
                   << "testRunning:" << m_testRunning;
        return;
    }

    m_currentMode = mode;
    m_testRunning = true;

    emit logMessage(QString("Запуск теста: %1").arg(m_currentMode));
    qDebug() << "ModeController: Starting test:" << m_currentMode;

    // Создаём сессию в репозитории
    if (m_dataRepository) {

        // Подключаемся к сигналу создания сессии
        connect(m_dataRepository, &IDataRepository::sessionCreated,
                this, &ModeController::onSessionCreated, Qt::UniqueConnection);

        m_dataRepository->setCurrentTestSession(m_currentMode);
        qDebug() << "ModeController: Test session created in repository";
    } else {
        qWarning() << "ModeController: DataRepository not set!";
    }
    writeModeRegister();
}

void ModeController::onSessionCreated(int sessionId) {
    qDebug() << "ModeController: Session created with ID:" << sessionId;

    // Теперь когда сессия создана, можно писать в D0
    writeModeRegister();
}

void ModeController::writeModeRegister() {
    // Запись в D0 согласно алгоритму
    if (m_client && m_client->isConnected()) {
        quint16 d0Value = 0;
        if (m_currentMode == "Расконсервация/Консервация") d0Value = 1;
        else if (m_currentMode == "Холодная прокрутка турбостартера") d0Value = 2;
        else if (m_currentMode == "Регулировка мощности, замер параметров") d0Value = 3;
        else if (m_currentMode == "Холодная прокрутка основного двигателя") d0Value = 4;
        else if (m_currentMode == "Имитация запуска основного двигателя") d0Value = 5;

        m_client->writeAndVerifyRegister(QModbusDataUnit::HoldingRegisters,
                                         DeltaAS332T::Addresses::D0_MODE_REGISTER, d0Value, 5000);

        emit logMessage(QString("[%1] Установка режима: %2 (D0=%3)")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                            .arg(m_currentMode)
                            .arg(d0Value));

        qDebug() << "ModeController: Mode register D0 set to" << d0Value;
    }

    emit modeChanged(m_currentMode);
}

void ModeController::onStopClicked() {
    qDebug() << "ModeController: Stop clicked";

    if (!m_testRunning) {
        qWarning() << "ModeController: No test running to stop";
        return;
    }

    // Send stop command via Modbus
    if (m_client && m_client->isConnected()) {
        m_client->writeRegister(QModbusDataUnit::HoldingRegisters,
                                DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);
    }

    // Сохраняем сессию перед остановкой
    if (m_dataRepository) {
        m_dataRepository->saveCurrentSessionToDatabase();
        qDebug() << "ModeController: Test session saved on stop";
    }

    m_testRunning = false;
    m_currentMode.clear();

    emit testStopped();
    emit logMessage(QString("[%1] Тест остановлен")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

    qDebug() << "ModeController: Test stopped by user";
}

void ModeController::onRegisterWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success) {
    if (address == DeltaAS332T::Addresses::D0_MODE_REGISTER) {
        if (success) {
            emit logMessage(QString("[%1] ✓ Режим установлен: D0=%2")
                                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                .arg(value));

            // После подтверждения записи D0 запускаем тест
            if (value > 0 && m_testRunning) {
                emit testStarted(m_currentMode);
                qDebug() << "ModeController: Test started successfully for mode:" << m_currentMode;
            }
        } else {
            emit logMessage(QString("[%1] ✗ Ошибка установки режима: D0 не установлен (прочитано: %2)")
                                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                .arg(value));

            // При ошибке возвращаем состояние
            m_testRunning = false;
            m_currentMode.clear();
            emit testStopped();

            qDebug() << "ModeController: Failed to start test for mode:" << m_currentMode;
        }
    }
}

void ModeController::stopTest() {
    if (m_testRunning) {
        qDebug() << "ModeController: Stopping test (forced)";

        // Сохраняем сессию перед остановкой
        if (m_dataRepository) {
            m_dataRepository->saveCurrentSessionToDatabase();
            qDebug() << "ModeController: Test session saved on forced stop";
        }

        m_testRunning = false;
        m_currentMode.clear();

        // Сбрасываем D0 в 0
        if (m_client && m_client->isConnected()) {
            m_client->writeRegister(QModbusDataUnit::HoldingRegisters,
                                    DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);
        }

        emit testStopped();
        emit logMessage(QString("[%1] Тест принудительно остановлен")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

        qDebug() << "ModeController: Test stopped by exit command";
    }
}

```

---


## Файл: ModbusClient/src/control/ControlStateMachine.cpp

```cpp
#include "ControlStateMachine.h"
#include "core/interfaces/IModbusClient.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QTimer>
#include <QDateTime>
#include <QFinalState>
#include <QSignalTransition>
#include <QDebug>


ControlStateMachine::ControlStateMachine(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_currentState(STATE_READY_CHECK)
    , m_stateMachine(new QStateMachine(this))
    , m_m11Ready(false)
    , m_m12Started(false)
    , m_m0Status(false)
    , m_m14Status(false)
{
    registerMetaTypes();
    setupStateMachine();
    setupStatusMonitoring(); // НОВЫЙ метод
}

ControlStateMachine::~ControlStateMachine() {
    if (m_stateMachine) {
        m_stateMachine->stop();
    }
}

void ControlStateMachine::registerMetaTypes() {
    qRegisterMetaType<ControlStateMachine::State>("ControlStateMachine::State");
}

void ControlStateMachine::setupStatusMonitoring() {
    if (!m_client) {
        qCritical() << "ControlStateMachine: IModbusClient is null!";
        return;
    }

    // КРИТИЧЕСКИ ВАЖНО: Подключаемся к сигналу чтения регистров
    connect(m_client, &IModbusClient::registerReadCompleted,
            this, &ControlStateMachine::onStatusRegisterRead);

    qDebug() << "ControlStateMachine: Status register monitoring configured";
}

void ControlStateMachine::onStatusRegisterRead(QModbusDataUnit::RegisterType type,
                                               quint16 address,
                                               quint16 value) {
    // Обрабатываем только Coils (M-регистры)
    if (type != QModbusDataUnit::Coils) {
        return;
    }

    // M11 - готовность к запуску
    if (address == DeltaAS332T::Addresses::M11_READY_STATUS) {
        bool ready = (value > 0);
        if (m_m11Ready != ready) {
            m_m11Ready = ready;
            if (ready && m_currentState == STATE_READY_CHECK) {
                qDebug() << "ControlStateMachine: M11=1 detected, transitioning to START_INTERRUPT";
                emit m11ReadySignal();
            }
        }
    }
    // M12 - тест запущен
    else if (address == DeltaAS332T::Addresses::M12_START_STATUS) {
        bool started = (value > 0);
        if (m_m12Started != started) {
            m_m12Started = started;
            if (started && m_currentState == STATE_START_INTERRUPT) {
                qDebug() << "ControlStateMachine: M12=1 detected, transitioning to STOP";
                emit m12StartedSignal();
            }
        }
    }
    // M0 - остановка
    else if (address == DeltaAS332T::Addresses::M0_STOP_STATUS) {
        bool stopped = (value > 0);
        if (m_m0Status != stopped) {
            m_m0Status = stopped;
            qDebug() << "ControlStateMachine: M0 changed to:" << stopped;
            checkCompletionCondition();
        }
    }
    // M14 - завершение
    else if (address == DeltaAS332T::Addresses::M14_COMPLETE_STATUS) {
        bool completed = (value > 0);
        if (m_m14Status != completed) {
            m_m14Status = completed;
            qDebug() << "ControlStateMachine: M14 changed to:" << completed;
            checkCompletionCondition();
        }
    }
}

void ControlStateMachine::checkCompletionCondition() {
    // Проверяем условие завершения только в состоянии STOP
    if (m_currentState == STATE_STOP) {
        bool completionCondition = m_m0Status && m_m14Status;

        if (completionCondition) {
            qDebug() << "ControlStateMachine: Completion condition met (M0=1 && M14=1)";
            emit completionSignal();
        }
    }
}

void ControlStateMachine::setupStateMachine() {
    // Создаем состояния
    m_readyCheckState = new QState();
    m_startInterruptState = new QState();
    m_stopState = new QState();
    m_restartExitState = new QState();

    // НАСТРАИВАЕМ ПРАВИЛЬНЫЕ ПЕРЕХОДЫ:

    // 1. STATE_READY_CHECK -> STATE_START_INTERRUPT (при M11 == 1)
    QSignalTransition* toStartInterruptTransition = new QSignalTransition(this, &ControlStateMachine::m11ReadySignal);
    toStartInterruptTransition->setTargetState(m_startInterruptState);
    m_readyCheckState->addTransition(toStartInterruptTransition);

    // 2. STATE_START_INTERRUPT -> STATE_STOP (при M12 == 1)
    QSignalTransition* toStopTransition = new QSignalTransition(this, &ControlStateMachine::m12StartedSignal);
    toStopTransition->setTargetState(m_stopState);
    m_startInterruptState->addTransition(toStopTransition);

    // 3. STATE_STOP -> STATE_RESTART_EXIT (при M0 == 1 && M14 == 1) - АВТОМАТИЧЕСКИЙ ПЕРЕХОД
    QSignalTransition* toRestartExitTransition = new QSignalTransition(this, &ControlStateMachine::completionSignal);
    toRestartExitTransition->setTargetState(m_restartExitState);
    m_stopState->addTransition(toRestartExitTransition);

    // 4. STATE_START_INTERRUPT -> STATE_READY_CHECK (при нажатии Прерывание)
    QSignalTransition* interruptTransition = new QSignalTransition(this, &ControlStateMachine::interruptTriggered);
    interruptTransition->setTargetState(m_readyCheckState);
    m_startInterruptState->addTransition(interruptTransition);

    // 5. STATE_STOP -> STATE_RESTART_EXIT при нажатии Стоп
    QSignalTransition* stopToRestartTransition = new QSignalTransition(this, &ControlStateMachine::stopTriggered);
    stopToRestartTransition->setTargetState(m_restartExitState);
    m_stopState->addTransition(stopToRestartTransition);

    // 6. STATE_RESTART_EXIT -> STATE_READY_CHECK (при нажатии Повторение запуска)
    QSignalTransition* restartTransition = new QSignalTransition(this, &ControlStateMachine::restartTriggered);
    restartTransition->setTargetState(m_readyCheckState);
    m_restartExitState->addTransition(restartTransition);

    // 7. STATE_READY_CHECK -> STATE_READY_CHECK (при нажатии Выход)
    QSignalTransition* exitFromReadyTransition = new QSignalTransition(this, &ControlStateMachine::exitTriggered);
    exitFromReadyTransition->setTargetState(m_readyCheckState);
    m_readyCheckState->addTransition(exitFromReadyTransition);

    // 8. STATE_RESTART_EXIT -> STATE_READY_CHECK (при нажатии Выход)
    QSignalTransition* exitFromRestartTransition = new QSignalTransition(this, &ControlStateMachine::exitTriggered);
    exitFromRestartTransition->setTargetState(m_readyCheckState);
    m_restartExitState->addTransition(exitFromRestartTransition);

    // Подключаем обработчики входа в состояния
    connect(m_readyCheckState, &QState::entered, this, &ControlStateMachine::transitionToReadyCheck);
    connect(m_startInterruptState, &QState::entered, this, &ControlStateMachine::transitionToStartInterrupt);
    connect(m_stopState, &QState::entered, this, &ControlStateMachine::transitionToStop);
    connect(m_restartExitState, &QState::entered, this, &ControlStateMachine::transitionToRestartExit);

    // Добавляем состояния в машину
    m_stateMachine->addState(m_readyCheckState);
    m_stateMachine->addState(m_startInterruptState);
    m_stateMachine->addState(m_stopState);
    m_stateMachine->addState(m_restartExitState);

    // Устанавливаем начальное состояние
    m_stateMachine->setInitialState(m_readyCheckState);
    m_stateMachine->start();

    qDebug() << "ControlStateMachine: State machine initialized and started";
}

void ControlStateMachine::transitionToReadyCheck() {
    m_currentState = STATE_READY_CHECK;

    // Сбрасываем статусы при возврате в начальное состояние
    m_m11Ready = false;
    m_m12Started = false;
    m_m0Status = false;
    m_m14Status = false;

    resetStatusRegisters();

    emit stateChanged(STATE_READY_CHECK);
    emit logMessage("Состояние: Проверка готовности");

    qDebug() << "ControlStateMachine: Transitioned to READY_CHECK";
}

void ControlStateMachine::transitionToStartInterrupt() {
    m_currentState = STATE_START_INTERRUPT;
    emit stateChanged(STATE_START_INTERRUPT);
    emit logMessage("Состояние: Пуск/Прерывание");

    qDebug() << "ControlStateMachine: Transitioned to START_INTERRUPT";
}

void ControlStateMachine::transitionToStop() {
    m_currentState = STATE_STOP;
    emit stateChanged(STATE_STOP);
    emit logMessage("Состояние: Стоп");

    qDebug() << "ControlStateMachine: Transitioned to STOP";
}

void ControlStateMachine::transitionToRestartExit() {
    m_currentState = STATE_RESTART_EXIT;
    emit stateChanged(STATE_RESTART_EXIT);

    // ОСТАНАВЛИВАЕМ ЗАПИСЬ ГРАФИКА ПРИ ПЕРЕХОДЕ В СОСТОЯНИЕ RESTART_EXIT
    emit stopChartRecording();
    emit logMessage("Состояние: Повторение/Выход - остановка записи графика");

    qDebug() << "ControlStateMachine: Transitioned to RESTART_EXIT";
}

// Public slots для внешних триггеров
void ControlStateMachine::triggerReadyCheck() {
    if (m_currentState == STATE_READY_CHECK) {
        writeM1ReadyCheck();
        emit readyCheckRequested();
    }
}

void ControlStateMachine::triggerStart() {
    if (m_currentState == STATE_START_INTERRUPT) {
        writeM2Start();
        emit startRequested();
        emit startTriggered(); // Сигнал для state machine
        // ЗАПУСКАЕМ ЗАПИСЬ ГРАФИКА ПРИ НАЖАТИИ ПУСК
        emit startChartRecording();
        emit logMessage("Запуск процесса - начало записи графика");
    }
}

void ControlStateMachine::triggerStop() {
    if (m_currentState == STATE_STOP) {
        writeM3Stop();
        emit stopRequested();
        emit stopTriggered(); // Сигнал для state machine
        emit logMessage("Ручная остановка - переход в состояние Повторение/Выход");
    }
}

void ControlStateMachine::triggerRestart() {
    if (m_currentState == STATE_RESTART_EXIT) {
        writeM4Restart();
        emit restartRequested();
        emit restartTriggered(); // Сигнал для state machine
    }
}

void ControlStateMachine::triggerInterrupt() {
    if (m_currentState == STATE_START_INTERRUPT) {
        writeM5Interrupt();
        emit interruptRequested();
        emit interruptTriggered(); // Сигнал для state machine
    }
}

void ControlStateMachine::triggerExit() {
    if (m_currentState == STATE_READY_CHECK || m_currentState == STATE_RESTART_EXIT) {
        emit stopChartRecording();
        // ЕСЛИ ТЕСТ ЗАПУЩЕН (не в состоянии READY_CHECK), ОСТАНАВЛИВАЕМ ЕГО
        if (m_currentState == STATE_RESTART_EXIT) {
            // Тест уже завершен или остановлен, просто выходим
            emit logMessage("Выход из завершенного теста");
        } else if (m_currentState == STATE_READY_CHECK) {
            // Если тест активен в фоне, останавливаем его
            emit stopCurrentTest();
            emit logMessage("Остановка теста и выход в меню");
        }
        resetStatusRegisters();
        writeM6Exit();
        emit exitRequested();
        emit exitTriggered();

        // СИГНАЛ ДЛЯ ОСТАНОВКИ ТЕСТА В MODE CONTROLLER
        emit stopCurrentTest();
    }
}

// Приватные методы для записи команд
void ControlStateMachine::resetStatusRegisters() {
    if (!m_client || !m_client->isConnected()) {
        qWarning() << "ControlStateMachine: Cannot reset status registers - device not connected";
        return;
    }

    // Список регистров для сброса
    QVector<QPair<QModbusDataUnit::RegisterType, quint16>> registersToReset = {
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS},
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS},
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS},
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS}
    };

    for (const auto& reg : registersToReset) {
        m_client->writeRegister(reg.first, reg.second, 0);
    }

    emit logMessage("Все статусные регистры сброшены в 0");
}

void ControlStateMachine::writeM1ReadyCheck() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M1_READY_CHECK,
                          1, "M1");
}

void ControlStateMachine::writeM2Start() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M2_START,
                          1, "M2");
}

void ControlStateMachine::writeM3Stop() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M3_STOP,
                          1, "M3");
}

void ControlStateMachine::writeM4Restart() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M4_RESTART,
                          1, "M4");
}

void ControlStateMachine::writeM5Interrupt() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M5_INTERRUPT,
                          1, "M5");
}

void ControlStateMachine::writeM6Exit() {
    if (!m_client || !m_client->isConnected()) {
        QString error = QString("[%1] Ошибка: устройство не подключено для выхода")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        emit logMessage(error);
        emit registerWriteVerified("M6", 1, false);
        emit registerWriteVerified("D0", 0, false);
        return;
    }

    // Логируем операцию выхода
    logRegisterOperation("Запись регистра",
                        DeltaAS332T::Addresses::M6_EXIT,
                        1, "M6");
    logRegisterOperation("Запись регистра",
                        DeltaAS332T::Addresses::D0_MODE_REGISTER,
                        0, "D0");

    // Используем верифицированную запись
    m_client->writeAndVerifyRegister(QModbusDataUnit::Coils,
                                   DeltaAS332T::Addresses::M6_EXIT, 1, 3000);
    m_client->writeAndVerifyRegister(QModbusDataUnit::HoldingRegisters,
                                   DeltaAS332T::Addresses::D0_MODE_REGISTER, 0, 3000);

    // Через секунду сбрасываем M6=0
    QTimer::singleShot(1000, [this]() {
        if (m_client && m_client->isConnected()) {
            logRegisterOperation("Сброс регистра",
                               DeltaAS332T::Addresses::M6_EXIT,
                               0, "M6");
            m_client->writeAndVerifyRegister(QModbusDataUnit::Coils,
                                           DeltaAS332T::Addresses::M6_EXIT, 0, 3000);
        }
    });
}

void ControlStateMachine::logRegisterOperation(const QString& operation, quint16 address,
                                              quint16 value, const QString& registerName, bool success) {
    QString status = success ? "✓ УСПЕХ" : "✗ ОШИБКА";
    QString message = QString("[%1] %2: %3=%4 (адрес: 0x%5) - %6")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                         .arg(operation)
                         .arg(registerName)
                         .arg(value)
                         .arg(address, 4, 16, QChar('0'))
                         .arg(status);
    emit logMessage(message);
    qDebug() << message;
}

void ControlStateMachine::writeAndVerifyRegister(QModbusDataUnit::RegisterType type,
                                                quint16 address, quint16 value,
                                                const QString& registerName) {
    if (!m_client || !m_client->isConnected()) {
        QString error = QString("[%1] Ошибка: устройство не подключено для записи %2")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                           .arg(registerName);
        emit logMessage(error);
        emit registerWriteVerified(registerName, value, false);
        return;
    }

    // Логируем начало операции
    logRegisterOperation("Запись регистра", address, value, registerName);

    // Используем верифицированную запись
    m_client->writeAndVerifyRegister(type, address, value, 3000);

    // Через секунду сбрасываем в 0 (для импульсных команд)
    QTimer::singleShot(1000, [this, type, address, registerName]() {
        if (m_client && m_client->isConnected()) {
            logRegisterOperation("Сброс регистра", address, 0, registerName);
            m_client->writeAndVerifyRegister(type, address, 0, 3000);
        }
    });
}

```

---


## Файл: ModbusClient/src/control/ControlUIController.cpp

```cpp
#include "ControlUIController.h"
#include "ControlStateMachine.h"
#include <QPushButton>
#include <QDebug>
#include <QTimer>

ControlUIController::ControlUIController(ControlStateMachine* stateMachine, QObject* parent)
    : QObject(parent)
    , m_stateMachine(stateMachine)
    , m_actionButton1(nullptr)
    , m_actionButton2(nullptr)
    , m_initialized(false)
{
    qDebug() << "ControlUIController created";

    if (m_stateMachine) {
        connect(m_stateMachine, &ControlStateMachine::stateChanged,
                this, &ControlUIController::onStateChanged, Qt::QueuedConnection); // Используем QueuedConnection
    }
}

void ControlUIController::setControlButtons(QPushButton* actionButton1, QPushButton* actionButton2) {
    qDebug() << "ControlUIController::setControlButtons called";

    // Защита от повторного вызова
    if (m_initialized) {
        qWarning() << "ControlUIController already initialized, ignoring duplicate call";
        return;
    }

    if (!actionButton1 || !actionButton2) {
        qCritical() << "ControlUIController: One or both buttons are null!";
        return;
    }

    // Проверяем, что кнопки еще "живы"
    if (actionButton1->isWidgetType() && actionButton2->isWidgetType()) {
        qDebug() << "Buttons are valid widgets";
    } else {
        qCritical() << "Buttons are not valid widgets!";
        return;
    }

    // Сохраняем новые кнопки
    m_actionButton1 = actionButton1;
    m_actionButton2 = actionButton2;

    qDebug() << "Buttons set successfully:"
             << "Button1:" << (void*)m_actionButton1
             << "Button2:" << (void*)m_actionButton2;

    // Безопасно отключаем ВСЕ существующие соединения от этих кнопок
    if (m_actionButton1) {
        try {
            m_actionButton1->disconnect();
            qDebug() << "Disconnected all signals from button1";
        } catch (...) {
            qCritical() << "Failed to disconnect button1";
            return;
        }
    }

    if (m_actionButton2) {
        try {
            m_actionButton2->disconnect();
            qDebug() << "Disconnected all signals from button2";
        } catch (...) {
            qCritical() << "Failed to disconnect button2";
            return;
        }
    }

    // Подключаем новые соединения с проверкой
    bool connect1 = false, connect2 = false;

    if (m_actionButton1) {
        connect1 = connect(m_actionButton1, &QPushButton::clicked,
                          this, &ControlUIController::onActionButton1Clicked,
                          Qt::QueuedConnection);
        qDebug() << "Button1 connection:" << (connect1 ? "success" : "failed");
    }

    if (m_actionButton2) {
        connect2 = connect(m_actionButton2, &QPushButton::clicked,
                          this, &ControlUIController::onActionButton2Clicked,
                          Qt::QueuedConnection);
        qDebug() << "Button2 connection:" << (connect2 ? "success" : "failed");
    }

    if (!connect1 || !connect2) {
        qCritical() << "Failed to connect one or both buttons";
        return;
    }

    // Помечаем как инициализированный
    m_initialized = true;

    // Откладываем обновление UI до следующего цикла событий
    QTimer::singleShot(0, this, [this]() {
        if (m_stateMachine && m_actionButton1 && m_actionButton2) {
            qDebug() << "Performing initial UI update";
            updateUIForState(m_stateMachine->currentState());
        } else {
            qCritical() << "Cannot perform initial UI update";
        }
    });

    qDebug() << "ControlUIController initialization completed successfully";
}

void ControlUIController::onStateChanged(ControlStateMachine::State newState) {
    qDebug() << "ControlUIController: State changed to" << newState;

    if (!m_actionButton1 || !m_actionButton2) {
        qWarning() << "ControlUIController: Buttons not available for UI update";
        return;
    }

    updateUIForState(newState);
    qDebug() << "UI update completed for state:" << newState;
}

void ControlUIController::updateUIForState(ControlStateMachine::State state) {
    qDebug() << "ControlUIController: Updating UI for state" << state;

    if (!m_actionButton1 || !m_actionButton2) {
        qCritical() << "ControlUIController: Cannot update UI - buttons are null";
        return;
    }

    // Временно блокируем сигналы чтобы избежать рекурсии
    m_actionButton1->blockSignals(true);
    m_actionButton2->blockSignals(true);

    try {
        // Сбрасываем стили
        m_actionButton1->setStyleSheet("");
        m_actionButton2->setStyleSheet("");

        switch (state) {
        case ControlStateMachine::STATE_READY_CHECK:
            qDebug() << "Setting state: READY_CHECK";
            m_actionButton1->setText("Проверка готовности");
            m_actionButton1->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #3498db; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #2980b9; }"
                "QPushButton:pressed { background-color: #21618c; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton1->setEnabled(true);
            m_actionButton1->setVisible(true);

            m_actionButton2->setText("Выход");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #e74c3c; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #c0392b; }"
                "QPushButton:pressed { background-color: #a93226; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;

        case ControlStateMachine::STATE_START_INTERRUPT:
            qDebug() << "Setting state: START_INTERRUPT";
            m_actionButton1->setText("ПУСК");
            m_actionButton1->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #27ae60; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #229954; }"
                "QPushButton:pressed { background-color: #1e8449; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton1->setEnabled(true);
            m_actionButton1->setVisible(true);

            m_actionButton2->setText("Прерывание\nзапуска");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #f39c12; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #d68910; }"
                "QPushButton:pressed { background-color: #b9770e; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;

        case ControlStateMachine::STATE_STOP:
            qDebug() << "Setting state: STOP";
            m_actionButton1->setText("");
            m_actionButton1->setStyleSheet("");
            m_actionButton1->setEnabled(false);
            m_actionButton1->setVisible(false);

            m_actionButton2->setText("СТОП");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #c0392b; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #a93226; }"
                "QPushButton:pressed { background-color: #922b21; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;

        case ControlStateMachine::STATE_RESTART_EXIT:
            qDebug() << "Setting state: RESTART_EXIT";
            m_actionButton1->setText("Повторение\nзапуска");
            m_actionButton1->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #9b59b6; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #8e44ad; }"
                "QPushButton:pressed { background-color: #7d3c98; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton1->setEnabled(true);
            m_actionButton1->setVisible(true);

            m_actionButton2->setText("Выход");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #e74c3c; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #c0392b; }"
                "QPushButton:pressed { background-color: #a93226; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;
        }

        // Принудительно обновляем отображение
        m_actionButton1->update();
        m_actionButton2->update();

        qDebug() << "UI update completed successfully";

    } catch (const std::exception& e) {
        qCritical() << "Exception in updateUIForState:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in updateUIForState";
    }

    // Разблокируем сигналы
    m_actionButton1->blockSignals(false);
    m_actionButton2->blockSignals(false);
}

void ControlUIController::onActionButton1Clicked() {
    qDebug() << "ControlUIController: Action button 1 clicked";

    if (!m_stateMachine) {
        qWarning() << "State machine not available";
        return;
    }

    ControlStateMachine::State currentState = m_stateMachine->currentState();
    qDebug() << "Current state:" << currentState;

    switch (currentState) {
    case ControlStateMachine::STATE_READY_CHECK:
        qDebug() << "Triggering ready check";
        m_stateMachine->triggerReadyCheck();
        break;
    case ControlStateMachine::STATE_START_INTERRUPT:
        qDebug() << "Triggering start";
        m_stateMachine->triggerStart();
        break;
    case ControlStateMachine::STATE_RESTART_EXIT:
        qDebug() << "Triggering restart";
        m_stateMachine->triggerRestart();
        break;
    case ControlStateMachine::STATE_STOP:
        qDebug() << "Button 1 inactive in STOP state";
        break;
    }
}

void ControlUIController::onActionButton2Clicked() {
    qDebug() << "ControlUIController: Action button 2 clicked";

    if (!m_stateMachine) {
        qWarning() << "State machine not available";
        return;
    }

    ControlStateMachine::State currentState = m_stateMachine->currentState();
    qDebug() << "Current state:" << currentState;

    switch (currentState) {
    case ControlStateMachine::STATE_READY_CHECK:
    case ControlStateMachine::STATE_RESTART_EXIT:
        qDebug() << "Triggering exit";
        m_stateMachine->triggerExit();
        break;
    case ControlStateMachine::STATE_START_INTERRUPT:
        qDebug() << "Triggering interrupt";
        m_stateMachine->triggerInterrupt();
        break;
    case ControlStateMachine::STATE_STOP:
        qDebug() << "Triggering stop";
        m_stateMachine->triggerStop();
        break;
    }
}

void ControlUIController::forceUpdateUI() {
    qDebug() << "ControlUIController: Force updating UI";
    if (m_stateMachine) {
        updateUIForState(m_stateMachine->currentState());
    }
}

```

---


## Файл: ModbusClient/src/control/ControlUIController.h

```cpp
#pragma once
#include <QObject>
#include "ControlStateMachine.h"  // Включаем полное определение

class QPushButton;

class ControlUIController : public QObject {
    Q_OBJECT
public:
    explicit ControlUIController(ControlStateMachine* stateMachine, QObject* parent = nullptr);

    void setControlButtons(QPushButton* actionButton1, QPushButton* actionButton2);
    inline bool isInitialized() const {return m_initialized;}

public slots:
    void onStateChanged(ControlStateMachine::State newState);
    void onActionButton1Clicked();
    void onActionButton2Clicked();
    void forceUpdateUI();

private:
    void updateUIForState(ControlStateMachine::State state);

    ControlStateMachine* m_stateMachine;
    QPushButton* m_actionButton1; // Кнопка для: Проверка готовности, Пуск, Повторение запуска
    QPushButton* m_actionButton2; // Кнопка для: Выход, Прерывание, Стоп
    bool m_initialized; // Защита от повторной инициализации
};

```

---


## Файл: ModbusClient/src/core/mapping/DeltaController.h

```cpp
#pragma once

namespace DeltaController {
    enum DiscreteInputs {
        S1 = 0, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12
    };

    enum CommandOutputs {
        K1 = 0, K2, K3, K4, K5, K6
    };

    enum AnalogValues {
        AD_RPM = 0,     // Частота вращения АД (об/мин)
        TK_RPM,         // Частота вращения ТК (об/мин)
        TK_PERCENT,     // Частота вращения ТК (%)
        ST_RPM,         // Частота вращения СТ (об/мин)
        ST_PERCENT      // Частота вращения СТ (%)
    };
}

```

---


## Файл: ModbusClient/src/core/mapping/DeltaAddressMapper.h

```cpp
#pragma once
#include "core/interfaces/IAddressMapper.h"
#include "DeltaController.h"
#include <QMap>

class DeltaAddressMapper : public IAddressMapper {
public:
    DeltaAddressMapper();
    ~DeltaAddressMapper() override = default;

    quint16 getDiscreteInputAddress(int input) const override;
    quint16 getCommandOutputAddress(int output) const override;
    quint16 getAnalogAddress(int analog) const override;
    QString getDiscreteInputName(int input) const override;
    QString getCommandOutputName(int output) const override;
    QString getAnalogName(int analog) const override;
    bool isValidAddress(quint16 address) const override;

private:
    void initializeMappings();

    QMap<int, quint16> m_discreteInputMap;
    QMap<int, quint16> m_commandOutputMap;
    QMap<int, quint16> m_analogMap;
    QMap<int, QString> m_discreteInputNames;
    QMap<int, QString> m_commandOutputNames;
    QMap<int, QString> m_analogNames;
};


```

---


## Файл: ModbusClient/src/core/mapping/DeltaAddressMap.h

```cpp
#pragma once
#include <QtGlobal>
#include <QVector>

namespace DeltaAS332T {
    namespace Addresses {
        // Discrete Inputs (X0.0 - X0.11) - Сигналы S1-S12
        const quint16 S1 = 0x6000;   // X0.0 Откл БУТС
        const quint16 S2 = 0x6001;   // X0.1 ОРТС
        const quint16 S3 = 0x6002;   // X0.2 ЭМЗС
        const quint16 S4 = 0x6003;   // X0.3 АЗТС
        const quint16 S5 = 0x6004;   // X0.4 ЭСТС
        const quint16 S6 = 0x6005;   // X0.5 Откл ЭСТС (СОЭС)
        const quint16 S7 = 0x6006;   // X0.6 ИП АЗТС Вкл.
        const quint16 S8 = 0x6007;   // X0.7 ИП ЭСТС Вкл.
        const quint16 S9 = 0x6008;   // X0.8 1й канал АЗТС Вкл.
        const quint16 S10 = 0x6009;  // X0.9 2й канал АЗТС Вкл.
        const quint16 S11 = 0x600A;  // X0.10 ЭСТС Вкл.
        const quint16 S12 = 0x600B;  // X0.11 ПЧ готов

        // Command Outputs (Y0.0 - Y0.8) - Команды K1-K6
        const quint16 K1 = 0xA000;   // Y0.0 Пуск ТС
        const quint16 K2 = 0xA001;   // Y0.1 Стоп ТС
        const quint16 K3 = 0xA002;   // Y0.2 СТОП-кран
        const quint16 K4 = 0xA003;   // Y0.3 Режим Консервации
        const quint16 K5 = 0xA004;   // Y0.4 Режим Холодной прокрутки
        const quint16 K6 = 0xA008;   // Y0.8 Активация Выходов ПЧ

        // Analog Values - регистры D (D0-D29999 = 0x0000-0x752F)
        const quint16 AD_RPM = 0x000A;      // D10 Частота вращения АД (об/мин)
        const quint16 TK_RPM = 0x0015;      // D21 (DWORD - читать 2 регистра) Частота вращения ТК (об/мин)
        const quint16 TK_PERCENT = 0x0017;  // D23 (DWORD - читать 2 регистра) Частота вращения ТК (%)
        const quint16 ST_RPM = 0x0029;      // D41 (DWORD - читать 2 регистра) Частота вращения СТ (об/мин)
        const quint16 ST_PERCENT = 0x002B;  // D43 (DWORD - читать 2 регистра) Частота вращения СТ (%)

        const quint16 SM_TEST = 0x404C; // SM76 bit флаг запроса отправки данных с Card1

        // / Control registers according to algorithm
        const quint16 D0_MODE_REGISTER = 0x0000;  // D0 - режим работы

        // M-registers for control buttons
        const quint16 M1_READY_CHECK = 0x0001;    // M1 - Проверка готовности
        const quint16 M2_START = 0x0002;          // M2 - ПУСК
        const quint16 M3_STOP = 0x0003;           // M3 - СТОП
        const quint16 M4_RESTART = 0x0004;        // M4 - Повторение запуска
        const quint16 M5_INTERRUPT = 0x0005;      // M5 - Прерывание запуска
        const quint16 M6_EXIT = 0x0006;           // M6 - ВЫХОД

        // Status registers
        const quint16 M0_STOP_STATUS = 0x0000;  // M0 - отсутсвие вращения
        const quint16 M11_READY_STATUS = 0x000B;  // M11 - статус готовности
        const quint16 M12_START_STATUS = 0x000C;  // M12 - статус запуска
        const quint16 M14_COMPLETE_STATUS = 0x000E; // M14 - завершение запуска
    }

    // Value conversion functions
    inline double scaleAnalogValue(quint16 raw, double factor = 1.0) {
        return raw * factor;
    }

    inline double scaleRPM(quint16 raw) {
        return scaleAnalogValue(raw, 1.0); // RPM в об/мин
    }

    inline double scalePercent(quint16 raw) {
        return scaleAnalogValue(raw, 1); // Проценты с коэффициентом 0.1
    }

    // Функции для работы с DWORD значениями (2 регистра)
    inline double readDWORD(const QVector<quint16>& values, int startIndex) {
        if (values.size() >= startIndex + 2) {
            quint32 result = (static_cast<quint32>(values[startIndex + 1]) << 16) | values[startIndex];
            return static_cast<double>(result);
        }
        return 0.0;
    }

    // Функция для записи DWORD значения
    inline QVector<quint16> writeDWORD(quint32 value) {
        QVector<quint16> result;
        result.append(static_cast<quint16>(value & 0xFFFF));
        result.append(static_cast<quint16>((value >> 16) & 0xFFFF));
        return result;
    }
}

```

---


## Файл: ModbusClient/src/core/mapping/DeltaAddressMapper.cpp

```cpp
#include "DeltaAddressMapper.h"
#include "DeltaAddressMap.h"

DeltaAddressMapper::DeltaAddressMapper() {
    initializeMappings();
}

void DeltaAddressMapper::initializeMappings() {
    // Discrete inputs mapping
    m_discreteInputMap[DeltaController::S1] = DeltaAS332T::Addresses::S1;
    m_discreteInputMap[DeltaController::S2] = DeltaAS332T::Addresses::S2;
    m_discreteInputMap[DeltaController::S3] = DeltaAS332T::Addresses::S3;
    m_discreteInputMap[DeltaController::S4] = DeltaAS332T::Addresses::S4;
    m_discreteInputMap[DeltaController::S5] = DeltaAS332T::Addresses::S5;
    m_discreteInputMap[DeltaController::S6] = DeltaAS332T::Addresses::S6;
    m_discreteInputMap[DeltaController::S7] = DeltaAS332T::Addresses::S7;
    m_discreteInputMap[DeltaController::S8] = DeltaAS332T::Addresses::S8;
    m_discreteInputMap[DeltaController::S9] = DeltaAS332T::Addresses::S9;
    m_discreteInputMap[DeltaController::S10] = DeltaAS332T::Addresses::S10;
    m_discreteInputMap[DeltaController::S11] = DeltaAS332T::Addresses::S11;
    m_discreteInputMap[DeltaController::S12] = DeltaAS332T::Addresses::S12;

    // Names
    m_discreteInputNames[DeltaController::S1] = "S1: Откл БУТС";
    m_discreteInputNames[DeltaController::S2] = "S2: ОРТС";
    m_discreteInputNames[DeltaController::S3] = "S3: ЭМЗС";
    m_discreteInputNames[DeltaController::S4] = "S4: АЗТС";
    m_discreteInputNames[DeltaController::S5] = "S5: ЭСТС";
    m_discreteInputNames[DeltaController::S6] = "S6: Откл ЭСТС (СОЭС)";
    m_discreteInputNames[DeltaController::S7] = "S7: ИП АЗТС Вкл.";
    m_discreteInputNames[DeltaController::S8] = "S8: ИП ЭСТС Вкл.";
    m_discreteInputNames[DeltaController::S9] = "S9: 1й канал АЗТС Вкл.";
    m_discreteInputNames[DeltaController::S10] = "S10: 2й канал АЗТС Вкл.";
    m_discreteInputNames[DeltaController::S11] = "S11: ЭСТС Вкл.";
    m_discreteInputNames[DeltaController::S12] = "S12: ПЧ готов";

    // Command outputs mapping
    m_commandOutputMap[DeltaController::K1] = DeltaAS332T::Addresses::K1;
    m_commandOutputMap[DeltaController::K2] = DeltaAS332T::Addresses::K2;
    m_commandOutputMap[DeltaController::K3] = DeltaAS332T::Addresses::K3;
    m_commandOutputMap[DeltaController::K4] = DeltaAS332T::Addresses::K4;
    m_commandOutputMap[DeltaController::K5] = DeltaAS332T::Addresses::K5;
    m_commandOutputMap[DeltaController::K6] = DeltaAS332T::Addresses::K6;

    m_commandOutputNames[DeltaController::K1] = "Пуск ТС";
    m_commandOutputNames[DeltaController::K2] = "Стоп ТС";
    m_commandOutputNames[DeltaController::K3] = "СТОП-кран";
    m_commandOutputNames[DeltaController::K4] = "Режим Консервации";
    m_commandOutputNames[DeltaController::K5] = "Режим Холодной прокрутки";
    m_commandOutputNames[DeltaController::K6] = "Активация Выходов ПЧ";

    // Analog values mapping
    m_analogMap[DeltaController::AD_RPM] = DeltaAS332T::Addresses::AD_RPM;
    m_analogMap[DeltaController::TK_RPM] = DeltaAS332T::Addresses::TK_RPM;
    m_analogMap[DeltaController::TK_PERCENT] = DeltaAS332T::Addresses::TK_PERCENT;
    m_analogMap[DeltaController::ST_RPM] = DeltaAS332T::Addresses::ST_RPM;
    m_analogMap[DeltaController::ST_PERCENT] = DeltaAS332T::Addresses::ST_PERCENT;

    m_analogNames[DeltaController::AD_RPM] = "Частота вращения АД (об/мин)";
    m_analogNames[DeltaController::TK_RPM] = "Частота вращения ТК (об/мин)";
    m_analogNames[DeltaController::TK_PERCENT] = "Частота вращения ТК (%)";
    m_analogNames[DeltaController::ST_RPM] = "Частота вращения СТ (об/мин)";
    m_analogNames[DeltaController::ST_PERCENT] = "Частота вращения СТ (%)";
}

quint16 DeltaAddressMapper::getDiscreteInputAddress(int input) const {
    return m_discreteInputMap.value(input, 0);
}

quint16 DeltaAddressMapper::getCommandOutputAddress(int output) const {
    return m_commandOutputMap.value(output, 0);
}

quint16 DeltaAddressMapper::getAnalogAddress(int analog) const {
    return m_analogMap.value(analog, 0);
}

QString DeltaAddressMapper::getDiscreteInputName(int input) const {
    return m_discreteInputNames.value(input, "Unknown");
}

QString DeltaAddressMapper::getCommandOutputName(int output) const {
    return m_commandOutputNames.value(output, "Unknown");
}

QString DeltaAddressMapper::getAnalogName(int analog) const {
    return m_analogNames.value(analog, "Unknown");
}

bool DeltaAddressMapper::isValidAddress(quint16 address) const {
    return m_discreteInputMap.values().contains(address) ||
           m_commandOutputMap.values().contains(address) ||
           m_analogMap.values().contains(address);
}

```

---


## Файл: ModbusClient/src/core/connection/ConnectionManager.h

```cpp
#pragma once
#include <QObject>
#include <QTimer>

class IModbusClient;

class ConnectionManager : public QObject {
    Q_OBJECT
public:
    explicit ConnectionManager(IModbusClient* client, QObject* parent = nullptr);
    ~ConnectionManager() override;

    void connectToDevice(const QString& address, quint16 port);
    void disconnectFromDevice();
    void setLocalPort(quint16 port);

    bool isConnected() const;
    QString connectionStatus() const;
    IModbusClient* modbusClient() const { return m_client; }

signals:
    void connectionStatusChanged(const QString& status);
    void logMessage(const QString& message);
    void errorOccurred(const QString& error);

private slots:
    void onModbusConnected();
    void onModbusDisconnected();
    void onModbusError(const QString& error);
    void onConnectionTimeout();

private:
    void resetConnectionAttempts();

    IModbusClient* m_client;
    QTimer* m_connectionTimer;

    bool m_connected;
    int m_connectionAttempts;
    QString m_currentAddress;
    quint16 m_currentPort;
};


```

---


## Файл: ModbusClient/src/core/connection/ConnectionManager.cpp

```cpp
#include "ConnectionManager.h"
#include "core/interfaces/IModbusClient.h"
#include "core/modbus/DeltaModbusClient.h"
#include <QDateTime>
#include <QDebug>

ConnectionManager::ConnectionManager(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_connectionTimer(new QTimer(this))
    , m_connected(false)
    , m_connectionAttempts(0)
{
    m_connectionTimer->setSingleShot(true);
    connect(m_connectionTimer, &QTimer::timeout,
            this, &ConnectionManager::onConnectionTimeout);

    connect(m_client, &IModbusClient::connected,
            this, &ConnectionManager::onModbusConnected);
    connect(m_client, &IModbusClient::disconnected,
            this, &ConnectionManager::onModbusDisconnected);
    connect(m_client, &IModbusClient::errorOccurred,
            this, &ConnectionManager::onModbusError);
}

ConnectionManager::~ConnectionManager() {
    m_connectionTimer->stop();
    disconnectFromDevice();
}

void ConnectionManager::connectToDevice(const QString& address, quint16 port) {
    if (m_connected) {
        emit logMessage("Already connected to device");
        return;
    }

    if (m_connectionAttempts >= 3) {
        emit errorOccurred("Too many connection attempts. Check device availability.");
        return;
    }

    m_connectionAttempts++;
    m_currentAddress = address;
    m_currentPort = port;

    emit logMessage(QString("[%1] Attempting connection to %2:%3 (attempt %4)")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                        .arg(address).arg(port).arg(m_connectionAttempts));

    m_connectionTimer->start(10000);

    // Установить порт сервера перед подключением
    if (auto deltaClient = qobject_cast<DeltaModbusClient*>(m_client)) {
        // Установка порта сервера через Modbus клиент
        m_client->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    }

    if (m_client->connectToDevice(address, port)) {
        emit logMessage("Connection initiated...");
    } else {
        m_connectionTimer->stop();
        emit errorOccurred("Failed to start connection process");
    }
}

void ConnectionManager::disconnectFromDevice() {
    m_connectionTimer->stop();
    m_connectionAttempts = 0;

    if (m_client) {
        m_client->disconnectFromDevice();
    }
}

void ConnectionManager::setLocalPort(quint16 port) {
    qDebug() << "ConnectionManager: Setting local port to" << port;

    // This should call the concrete implementation method
    // We'll need to cast to DeltaModbusClient if needed
    emit logMessage(QString("Setting local port to %1").arg(port));
}

bool ConnectionManager::isConnected() const {
    return m_connected;
}

QString ConnectionManager::connectionStatus() const {
    return m_connected ? "ПОДКЛЮЧЕН" : "ОТКЛЮЧЕН";
}

void ConnectionManager::onModbusConnected() {
    m_connectionTimer->stop();
    m_connected = true;
    m_connectionAttempts = 0;

    emit connectionStatusChanged("ПОДКЛЮЧЕН");
    emit logMessage(QString("[%1] Successfully connected to device")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void ConnectionManager::onModbusDisconnected() {
    m_connectionTimer->stop();
    m_connected = false;

    emit connectionStatusChanged("ОТКЛЮЧЕН");
    emit logMessage(QString("[%1] Disconnected from device")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void ConnectionManager::onModbusError(const QString& error) {
    m_connectionTimer->stop();

    emit errorOccurred(error);
    emit logMessage(QString("[%1] ERROR: %2")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                        .arg(error));

    if (error.contains("timeout", Qt::CaseInsensitive) ||
        error.contains("connection refused", Qt::CaseInsensitive)) {
        disconnectFromDevice();
    }
}

void ConnectionManager::onConnectionTimeout() {
    if (!m_connected) {
        m_connectionAttempts++;
        emit errorOccurred(QString("Connection timeout after 10 seconds to %1:%2")
                               .arg(m_currentAddress).arg(m_currentPort));

        if (m_client) {
            m_client->disconnectFromDevice();
        }

        if (m_connectionAttempts < 3) {
            emit logMessage("Retrying connection...");
            QTimer::singleShot(2000, this, [this]() {
                connectToDevice(m_currentAddress, m_currentPort);
            });
        }
    }
}

void ConnectionManager::resetConnectionAttempts() {
    m_connectionAttempts = 0;
}


```

---


## Файл: ModbusClient/src/core/interfaces/IIndicator.h

```cpp
#ifndef IINDICATOR_H
#define IINDICATOR_H

#include <QObject>

class IValueIndicator {
public:
    virtual ~IValueIndicator() = default;
    virtual void setValue(double value) = 0;
    virtual void setRange(double min, double max) = 0;
//    virtual void setVisible(bool visible) = 0;
};

class IDualIndicator : public IValueIndicator {
public:
    virtual ~IDualIndicator() = default;
    virtual void setSecondaryValue(double value) = 0;
    virtual void setSecondaryRange(double min, double max) = 0;
};

#endif // IINDICATOR_H

```

---


## Файл: ModbusClient/src/core/interfaces/IRequestQueue.h

```cpp
#pragma once
#include <QObject>
#include <functional>
#include <QModbusDataUnit>

enum class RequestType {
    Read,
    Write
};

struct ModbusRequest {
    RequestType type;
    QModbusDataUnit::RegisterType registerType;
    quint16 address;
    quint16 count; // for read operations
    quint16 value; // for write operations
    QString parameterName;
    std::function<void(bool, const QString&)> callback;

    ModbusRequest() = default;

    ModbusRequest(RequestType t, QModbusDataUnit::RegisterType rt, quint16 addr,
                  quint16 cnt = 1, quint16 val = 0, const QString& param = "")
        : type(t), registerType(rt), address(addr), count(cnt), value(val), parameterName(param)
    {}
};

class IRequestQueue : public QObject {
    Q_OBJECT
public:
    explicit IRequestQueue(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IRequestQueue() = default;

    virtual void enqueueRead(QModbusDataUnit::RegisterType type, quint16 address,
                            quint16 count = 1, const QString& paramName = "") = 0;
    virtual void enqueuePriorityRead(QModbusDataUnit::RegisterType type, quint16 address,
                                     quint16 count, const QString& paramName = "") = 0;

    virtual void enqueueWrite(QModbusDataUnit::RegisterType type, quint16 address,
                             quint16 value) = 0;
    virtual bool hasRequests() const = 0;
    virtual ModbusRequest dequeue() = 0;
    virtual void clear() = 0;
    virtual int size() const = 0;

signals:
    void requestAdded();
};

```

---


## Файл: ModbusClient/src/core/interfaces/IAddressMapper.h

```cpp
#pragma once
#include <QString>
#include <QMap>

    class IAddressMapper {
public:
    virtual ~IAddressMapper() = default;

    virtual quint16 getDiscreteInputAddress(int input) const = 0;
    virtual quint16 getCommandOutputAddress(int output) const = 0;
    virtual quint16 getAnalogAddress(int analog) const = 0;
    virtual QString getDiscreteInputName(int input) const = 0;
    virtual QString getCommandOutputName(int output) const = 0;
    virtual QString getAnalogName(int analog) const = 0;
    virtual bool isValidAddress(quint16 address) const = 0;
};


```

---


## Файл: ModbusClient/src/core/interfaces/IModbusClient.h

```cpp
#pragma once
#include <QObject>
#include <QVector>
#include <QVariant>
#include <QModbusDataUnit>
#include <QModbusDevice>

class IModbusClient : public QObject {
    Q_OBJECT
public:
    explicit IModbusClient(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IModbusClient() = default;

    virtual void setConnectionParameter(QModbusDevice::ConnectionParameter param, const QVariant& value) = 0;

    // Connection management
    virtual bool connectToDevice(const QString& address, quint16 port) = 0;
    virtual void disconnectFromDevice() = 0;
    virtual bool isConnected() const = 0;

    // New unified interface with register type
    virtual void readRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 count = 1) = 0;
    virtual void writeRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) = 0;

    virtual void writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, int timeoutMs = 3000)=0;

    enum PollingFrequency {
        HighFrequency = 20,  // 20 Hz
        LowFrequency = 2     // 2 Hz
    };

    // virtual void setPollingInterval(int intervalMs) = 0;
    // virtual void addPolledRegister(const QString& name, QModbusDataUnit::RegisterType type, quint16 address, quint16 count = 1) = 0;

    // Polling configuration with register type
    virtual void addPolledRegisterWithFrequency(const QString& name,
                                                QModbusDataUnit::RegisterType type,
                                                quint16 address,
                                                quint16 count = 1,
                                                PollingFrequency frequency = LowFrequency) = 0;
    virtual void removePolledRegister(const QString& name) = 0;
    virtual void clearPolledRegisters() = 0;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void dataRead(const QString& parameter, const QVariant& value);
    void registerReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);
    void registersReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values);
    void registerWriteCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success);
    void registerWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success);
};

```

---


## Файл: ModbusClient/src/core/interfaces/IPollingConfigurator.h

```cpp
#pragma once
#include <QObject>

class IPollingConfigurator {
public:
    virtual ~IPollingConfigurator() = default;

    virtual void setHighFrequencyInterval(int ms) = 0;
    virtual void setLowFrequencyInterval(int ms) = 0;
    virtual void addToHighFrequencyGroup(const QString& name) = 0;
    virtual void addToLowFrequencyGroup(const QString& name) = 0;
    virtual void removeFromPolling(const QString& name) = 0;
};

```

---


## Файл: ModbusClient/src/core/modbus/ModbusRequestQueue.h

```cpp
#pragma once
#include "core/interfaces/IRequestQueue.h"
#include <QQueue>
#include <QMutex>

class ModbusRequestQueue : public IRequestQueue {
    Q_OBJECT
public:
    explicit ModbusRequestQueue(QObject* parent = nullptr);
    ~ModbusRequestQueue() override = default;

    void enqueueRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 count, const QString& paramName) override;
    void enqueueWrite(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) override;
    void enqueuePriorityRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 count, const QString& paramName) override;
    bool hasRequests() const override;
    ModbusRequest dequeue() override;
    void clear() override;
    int size() const override;

private:
    mutable QMutex m_mutex;
    QQueue<ModbusRequest> m_priorityQueue; // Приоритетная очередь для записей и проверок
    QQueue<ModbusRequest> m_normalQueue;   // Обычная очередь для опросов
};

```

---


## Файл: ModbusClient/src/core/modbus/CustomModbusClient.cpp

```cpp
#include "CustomModbusClient.h"
#include <QHostAddress>
#include <QDebug>

CustomModbusClient::CustomModbusClient(QObject* parent)
    : QModbusTcpClient(parent)
    , m_customSocket(nullptr)
{}

bool CustomModbusClient::bindToPort(quint16 port) {
    // Для QModbusTcpClient установка локального порта происходит через параметры соединения
    // перед подключением, а не через привязку сокета

    qDebug() << "Setting up Modbus client for local port:" << port;

    // Устанавливаем локальный порт через параметры соединения
    setConnectionParameter(QModbusDevice::NetworkPortParameter, port);

    return true;
}

```

---


## Файл: ModbusClient/src/core/modbus/factoies/PollingConfiguratorFactory.h

```cpp
#pragma once
#include "core/interfaces/IModbusClient.h"
#include "core/interfaces/IPollingConfigurator.h"

class PollingConfiguratorFactory {
public:
    static void configureDefaultPolling(IModbusClient* client);
};

```

---


## Файл: ModbusClient/src/core/modbus/factoies/PollingConfiguratorFactory.cpp

```cpp
#include "PollingConfiguratorFactory.h"
#include "core/mapping/DeltaAddressMap.h"

void PollingConfiguratorFactory::configureDefaultPolling(IModbusClient *client)
{
    if (!client) return;

    // Настройка высокочастотных каналов (20 Гц)
    client->addPolledRegisterWithFrequency("AD_RPM",
                                           QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::AD_RPM, 1,
                                           IModbusClient::HighFrequency);
    client->addPolledRegisterWithFrequency("TK_RPM",
                                           QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::TK_RPM, 2,
                                           IModbusClient::HighFrequency);
    client->addPolledRegisterWithFrequency("ST_RPM",
                                           QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::ST_RPM, 2,
                                           IModbusClient::HighFrequency);

    // Настройка низкочастотных каналов (2 Гц)
    client->addPolledRegisterWithFrequency("S1",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S1, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S2",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S2, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S3",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S3, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S4",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S4, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S5",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S5, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S6",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S6, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S7",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S7, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S8",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S8, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S9",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S9, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S10",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S10, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S11",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S11, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S12",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S12, 1,
                                           IModbusClient::LowFrequency);

    client->addPolledRegisterWithFrequency("K1",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K1, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K2",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K2, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K3",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K3, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K4",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K4, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K5",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K5, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K6",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K6, 1,
                                           IModbusClient::LowFrequency);

    client->addPolledRegisterWithFrequency("M0_STOP_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("M11_READY_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("M12_START_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("M14_COMPLETE_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS, 1,
                                           IModbusClient::LowFrequency);
}

```

---


## Файл: ModbusClient/src/core/modbus/DeltaModbusClient.h

```cpp
#pragma once
#include "core/interfaces/IModbusClient.h"
#include "core/interfaces/IRequestQueue.h"
#include "core/interfaces/IAddressMapper.h"
#include <QTimer>
#include <QModbusDevice>
#include <QScopedPointer>
#include <QDateTime>

class CustomModbusClient;
class ModbusRequestHandler;

class DeltaModbusClient : public IModbusClient {
    Q_OBJECT
public:
    explicit DeltaModbusClient(QObject* parent = nullptr);
    ~DeltaModbusClient() override;

    void setConnectionParameter(QModbusDevice::ConnectionParameter param, const QVariant& value) override;

    // IModbusClient interface
    bool connectToDevice(const QString& address, quint16 port) override;
    void disconnectFromDevice() override;
    bool isConnected() const override;

    // New unified methods with register type
    void readRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 count = 1) override;
    void writeRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) override;

    // Verification method
    void writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, int timeoutMs = 3000) override;

    // Polling configuration
    // void setPollingInterval(int intervalMs) override;
    void removePolledRegister(const QString& name) override;
    void clearPolledRegisters() override;

    void addPolledRegisterWithFrequency(const QString& name,
                                        QModbusDataUnit::RegisterType type,
                                        quint16 address,
                                        quint16 count = 1,
                                        PollingFrequency frequency = LowFrequency) override;

    // Delta-specific methods
    void setLocalPort(quint16 port);
    void setOperationMode(const QString& mode);
    QString currentOperationMode() const { return m_currentMode; }

private slots:
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);
    // void onPollTimeout();
    void onReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, const QString& paramName);
    void onReadsCompleted(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values);
    void onWriteCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success);
    void onRequestFailed(const QString& error);
    void onVerificationTimeout();

private:
    // Polling groups
    enum PollingGroup {
        HIGH_FREQUENCY,  // 20 Hz - analog channels
        LOW_FREQUENCY    // 2 Hz - all others
    };

    void initializeD0();
    void setupPollingTimers();
    void setupPollingGroups();
    void pollGroup(PollingGroup group);

    struct PolledRegister {
        QModbusDataUnit::RegisterType type;
        quint16 address;
        quint16 count;

        PolledRegister() : type(QModbusDataUnit::HoldingRegisters), address(0) {}
        PolledRegister(QModbusDataUnit::RegisterType t, quint16 addr, quint16 cnt = 1) : type(t), address(addr), count(cnt) {}
    };

    struct VerificationRequest {
        QModbusDataUnit::RegisterType type;
        quint16 address;
        quint16 expectedValue;
        QDateTime timestamp;
    };

    QMap<PollingGroup, QTimer*> m_pollTimers;
    QMap<PollingGroup, QMap<QString, PolledRegister>> m_pollingGroups;

    QScopedPointer<CustomModbusClient> m_client;
    QScopedPointer<IRequestQueue> m_queue;
    QScopedPointer<ModbusRequestHandler> m_handler;
    QScopedPointer<IAddressMapper> m_mapper;

    QTimer* m_verificationTimer;
    // QMap<QString, PolledRegister> m_polledRegisters;
    QMap<quint16, VerificationRequest> m_verificationRequests;
    QString m_currentMode;
    quint16 m_localPort;
    bool m_initialD0Written;
};

```

---


## Файл: ModbusClient/src/core/modbus/ModbusRequestHandler.cpp

```cpp
#include "ModbusRequestHandler.h"
#include "core/interfaces/IRequestQueue.h"
#include <QModbusDataUnit>
#include <QDebug>

ModbusRequestHandler::ModbusRequestHandler(QModbusTcpClient* client,
                                           IRequestQueue* queue,
                                           QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_queue(queue)
    , m_processTimer(new QTimer(this))
    , m_processing(false)
    , m_minRequestInterval(50)
{
    m_processTimer->setInterval(m_minRequestInterval);
    connect(m_processTimer, &QTimer::timeout, this, &ModbusRequestHandler::processNextRequest);
}

ModbusRequestHandler::~ModbusRequestHandler() {
    stop();
    if (m_processTimer) {
        m_processTimer->stop();
    }
}

void ModbusRequestHandler::setProcessInterval(int ms) {
    m_minRequestInterval = qMax(10, ms);
    m_processTimer->setInterval(m_minRequestInterval);
}

void ModbusRequestHandler::start() {
    if (!m_processTimer->isActive()) {
        m_processTimer->start();
    }
}

void ModbusRequestHandler::stop() {
    m_processTimer->stop();
    m_processing = false;
}

void ModbusRequestHandler::processNextRequest() {
//    qDebug() << "RequestHandler: processNextRequest called - Processing:" << m_processing;

    if (m_processing) {
//        qDebug() << "RequestHandler: Already processing, skipping";
        return;
    }

    if (!m_queue->hasRequests()) {
//        qDebug() << "RequestHandler: No requests in queue";
        return;
    }

    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
//        qDebug() << "RequestHandler: Client not ready - State:" << (m_client ? m_client->state() : -1);
        return;
    }

    m_processing = true;
    ModbusRequest request = m_queue->dequeue();

    if (request.type == RequestType::Read) {
//        qDebug() << "RequestHandler: Processing READ request";
        sendReadRequest(request.registerType, request.address, request.count, request.parameterName);
    } else {
//        qDebug() << "RequestHandler: Processing WRITE request";
        sendWriteRequest(request.registerType, request.address, request.value);
    }
}

void ModbusRequestHandler::sendReadRequest(QModbusDataUnit::RegisterType type,
                                         quint16 address, quint16 count,
                                         const QString& paramName) {
    QModbusDataUnit readUnit(type, address, count);
    QModbusReply* reply = m_client->sendReadRequest(readUnit, 1);

    if (!reply) {
        m_processing = false;
        emit requestFailed("Failed to create read request");
        return;
    }

    reply->setProperty("address", address);
    reply->setProperty("count", count);
    reply->setProperty("paramName", paramName);
    reply->setProperty("registerType", static_cast<int>(type));

    connect(reply, &QModbusReply::finished, this, &ModbusRequestHandler::handleReadReply);
}

void ModbusRequestHandler::sendWriteRequest(QModbusDataUnit::RegisterType type,
                                            quint16 address, quint16 value) {
//    qDebug() << "ModbusRequestHandler: Sending write request - Type:" << type
//             << "Address: 0x" << QString::number(address, 16)
//             << "Value:" << value
//             << "Client state:" << m_client->state();

    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        QString errorMsg = "Client not in connected state: " + QString::number(m_client->state());
//        qDebug() << "ModbusRequestHandler:" << errorMsg;
        m_processing = false;
        emit requestFailed(errorMsg);
        return;
    }

    QModbusDataUnit writeUnit(type, address, 1);

    // Для битовых регистров преобразуем значение
    if (type == QModbusDataUnit::Coils || type == QModbusDataUnit::DiscreteInputs) {
        writeUnit.setValue(0, value > 0 ? true : false);
//        qDebug() << "Coil write - setting value to:" << (value > 0 ? "true" : "false");
    } else {
        writeUnit.setValue(0, value);
//        qDebug() << "Register write - setting value to:" << value;
    }

    QModbusReply* reply = m_client->sendWriteRequest(writeUnit, 1);
//    qDebug() << "RequestHandler: Write request sent, reply:" << reply;

    if (!reply) {
        m_processing = false;
        QString errorMsg = "Failed to create write request for address: 0x" + QString::number(address, 16);
//        qDebug() << "ModbusRequestHandler:" << errorMsg;
        emit requestFailed(errorMsg);
        return;
    }

    reply->setProperty("address", address);
    reply->setProperty("value", value);
    reply->setProperty("registerType", static_cast<int>(type));

    connect(reply, &QModbusReply::finished, this, &ModbusRequestHandler::handleWriteReply);
    connect(reply, &QModbusReply::errorOccurred, this, [this, address, reply](QModbusDevice::Error error) {
        qDebug() << "ModbusRequestHandler: Write error for address 0x" << QString::number(address, 16)
                 << "Error:" << error << "Error string:" << reply->errorString();
    });

//    qDebug() << "ModbusRequestHandler: Write request sent successfully, reply:" << reply;
}

void ModbusRequestHandler::handleReadReply() {
    QModbusReply* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    quint16 address = reply->property("address").toUInt();
    quint16 count = reply->property("count").toUInt();
    QString paramName = reply->property("paramName").toString();
    QModbusDataUnit::RegisterType registerType = static_cast<QModbusDataUnit::RegisterType>(
        reply->property("registerType").toInt());

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();

        // Для всех типов регистров возвращаем quint16 значения
        if (count == 1) {
            quint16 value = unit.value(0);
            emit readCompleted(registerType, address, value, paramName);
        } else {
            QVector<quint16> values;
            for (quint16 i = 0; i < unit.valueCount(); ++i) {
                values.append(unit.value(i));
            }
            emit readsCompleted(registerType, address, values);
        }
    } else {
        emit requestFailed("Read error: " + reply->errorString());
    }

    reply->deleteLater();
    m_processing = false;
}

void ModbusRequestHandler::handleWriteReply() {
    QModbusReply* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) {
//        qDebug() << "RequestHandler: handleWriteReply - No reply object!";
        m_processing = false;
        return;
    }

    quint16 address = reply->property("address").toUInt();
    QModbusDataUnit::RegisterType registerType = static_cast<QModbusDataUnit::RegisterType>(
        reply->property("registerType").toInt());
    bool success = (reply->error() == QModbusDevice::NoError);

    emit writeCompleted(registerType, address, success);

    if (!success) {
        QString errorMsg = "Write error: " + reply->errorString();
//        qDebug() << "RequestHandler:" << errorMsg;
        emit requestFailed(errorMsg);
    }

    reply->deleteLater();
    m_processing = false;
//    qDebug() << "RequestHandler: Write processing completed";
}

```

---


## Файл: ModbusClient/src/core/modbus/CustomModbusClient.h

```cpp
#pragma once
#include <QModbusTcpClient>
#include <QTcpSocket>

class CustomModbusClient : public QModbusTcpClient {
    Q_OBJECT
public:
    explicit CustomModbusClient(QObject* parent = nullptr);
    bool bindToPort(quint16 port);

private:
    QTcpSocket* m_customSocket;
};


```

---


## Файл: ModbusClient/src/core/modbus/ModbusRequestQueue.cpp

```cpp
#include "ModbusRequestQueue.h"
#include <QMutexLocker>
#include <QDebug>

ModbusRequestQueue::ModbusRequestQueue(QObject* parent)
    : IRequestQueue(parent)
{}

void ModbusRequestQueue::enqueueRead(QModbusDataUnit::RegisterType type, quint16 address,
                                   quint16 count, const QString& paramName) {
    QMutexLocker locker(&m_mutex);

    ModbusRequest request(RequestType::Read, type, address, count, 0, paramName);
    m_normalQueue.enqueue(request);
//    qDebug() << "Queue: Enqueued READ - Type:" << type
//             << "Address: 0x" << QString::number(address, 16)
//             << "Count:" << count
//             << "Queue size:" << m_normalQueue.size();
    locker.unlock();
    emit requestAdded();
}

void ModbusRequestQueue::enqueueWrite(QModbusDataUnit::RegisterType type, quint16 address,
                                    quint16 value) {
    QMutexLocker locker(&m_mutex);

    ModbusRequest request(RequestType::Write, type, address, 1, value);
    m_priorityQueue.enqueue(request);
//    qDebug() << "Queue: Enqueued WRITE - Type:" << type
//             << "Address: 0x" << QString::number(address, 16)
//             << "Value:" << value
//             << "Queue size:" << m_priorityQueue.size();

    locker.unlock();
    emit requestAdded();
}

void ModbusRequestQueue::enqueuePriorityRead(QModbusDataUnit::RegisterType type, quint16 address,
                                           quint16 count, const QString& paramName) {
    QMutexLocker locker(&m_mutex);

    ModbusRequest request(RequestType::Read, type, address, count, 0, paramName);
    m_priorityQueue.enqueue(request); // Приоритетные чтения в приоритетную очередь

    locker.unlock();
    emit requestAdded();
}

bool ModbusRequestQueue::hasRequests() const {
    QMutexLocker locker(&m_mutex);
    bool hasRequests = !m_priorityQueue.isEmpty()|| !m_normalQueue.isEmpty();
//    qDebug() << "m_priorityQueue: hasRequests =" << hasRequests << "Size:" << m_priorityQueue.size();
//    qDebug() << "m_normalQueue: hasRequests =" << hasRequests << "Size:" << m_normalQueue.size();
    return hasRequests;
}

ModbusRequest ModbusRequestQueue::dequeue() {
    QMutexLocker locker(&m_mutex);

    if (m_priorityQueue.isEmpty()) {
//        qDebug() << "Queue: Attempt to dequeue from empty m_priorityQueue!";
        ModbusRequest request = m_normalQueue.dequeue();
//        qDebug() << "m_normalQueue: Dequeued - Type:" << (request.type == RequestType::Read ? "READ" : "WRITE")
//                 << "Address: 0x" << QString::number(request.address, 16)
//                 << "Remaining:" << m_normalQueue.size();
        return request;
    }
    else {
        ModbusRequest request = m_priorityQueue.dequeue();
//        qDebug() << "Queue: Dequeued - Type:" << (request.type == RequestType::Read ? "READ" : "WRITE")
//                 << "Address: 0x" << QString::number(request.address, 16)
//                 << "Remaining:" << m_priorityQueue.size();
        return request;
    }
}

void ModbusRequestQueue::clear() {
    QMutexLocker locker(&m_mutex);
    int previousSize = m_priorityQueue.size();
    m_priorityQueue.clear();
//    qDebug() << "m_priorityQueue: Cleared - Previous size:" << previousSize;

    previousSize = m_normalQueue.size();
    m_normalQueue.clear();
//    qDebug() << "m_priorityQueue: Cleared - Previous size:" << previousSize;
}

int ModbusRequestQueue::size() const {
    QMutexLocker locker(&m_mutex);
    return m_priorityQueue.size() + m_normalQueue.size();
}

```

---


## Файл: ModbusClient/src/core/modbus/DeltaModbusClient.cpp

```cpp
#include "DeltaModbusClient.h"
#include "CustomModbusClient.h"
#include "ModbusRequestQueue.h"
#include "ModbusRequestHandler.h"
#include "core/mapping/DeltaAddressMapper.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>

DeltaModbusClient::DeltaModbusClient(QObject* parent)
    : IModbusClient(parent)
    , m_client(new CustomModbusClient(this))
    , m_queue(new ModbusRequestQueue(this))
    , m_handler(new ModbusRequestHandler(m_client.data(), m_queue.data(), this))
    , m_mapper(new DeltaAddressMapper())
    , m_verificationTimer(new QTimer(this))
    , m_currentMode("Холодная прокрутка турбостартера")
    , m_localPort(3201)
    , m_initialD0Written(false)
{
    // Configure Modbus client
    m_client->setTimeout(5000);
    m_client->setNumberOfRetries(2);

    setupPollingTimers();
    setupPollingGroups();

    // Connect client signals
    connect(m_client.data(), &QModbusClient::stateChanged,
            this, &DeltaModbusClient::onStateChanged);
    connect(m_client.data(), &QModbusClient::errorOccurred,
            this, &DeltaModbusClient::onErrorOccurred);

    // Connect handler signals
    connect(m_handler.data(), SIGNAL(readCompleted(QModbusDataUnit::RegisterType,quint16,quint16,QString)),
            this, SLOT(onReadCompleted(QModbusDataUnit::RegisterType,quint16,quint16,QString)));
    connect(m_handler.data(), SIGNAL(readsCompleted(QModbusDataUnit::RegisterType,quint16,QVector<quint16>)),
            this, SLOT(onReadsCompleted(QModbusDataUnit::RegisterType,quint16,QVector<quint16>)));
    connect(m_handler.data(), SIGNAL(writeCompleted(QModbusDataUnit::RegisterType,quint16,bool)),
            this, SLOT(onWriteCompleted(QModbusDataUnit::RegisterType,quint16,bool)));

    connect(m_handler.data(), &ModbusRequestHandler::requestFailed,
            this, &DeltaModbusClient::onRequestFailed);

    // Verification timer
    m_verificationTimer->setInterval(500);
    connect(m_verificationTimer, &QTimer::timeout, this, &DeltaModbusClient::onVerificationTimeout);

    m_initialD0Written = false;
}

DeltaModbusClient::~DeltaModbusClient() {
    // Останавливаем все таймеры опроса
    for (QTimer* timer : m_pollTimers) {
        if (timer) {
            timer->stop();
        }
    }

    if (m_handler) {
        m_handler->stop();
    }

    disconnectFromDevice();
}

void DeltaModbusClient::setupPollingTimers() {
    // High frequency timer (20 Hz = 50 ms)
    m_pollTimers[HIGH_FREQUENCY] = new QTimer(this);
    m_pollTimers[HIGH_FREQUENCY]->setInterval(50);
    connect(m_pollTimers[HIGH_FREQUENCY], &QTimer::timeout,
            this, [this]() { pollGroup(HIGH_FREQUENCY); });

    // Low frequency timer (2 Hz = 500 ms)
    m_pollTimers[LOW_FREQUENCY] = new QTimer(this);
    m_pollTimers[LOW_FREQUENCY]->setInterval(500);
    connect(m_pollTimers[LOW_FREQUENCY], &QTimer::timeout,
            this, [this]() { pollGroup(LOW_FREQUENCY); });
}

void DeltaModbusClient::setupPollingGroups() {
    // High frequency group - analog values (20 Hz)
    m_pollingGroups[HIGH_FREQUENCY]["AD_RPM"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::AD_RPM, 1);
    m_pollingGroups[HIGH_FREQUENCY]["TK_RPM"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::TK_RPM, 2);
    m_pollingGroups[HIGH_FREQUENCY]["ST_RPM"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::ST_RPM, 2);

    // Low frequency group - discrete inputs and command outputs (2 Hz)
    m_pollingGroups[LOW_FREQUENCY]["TK_PERCENT"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::TK_PERCENT, 2);
    m_pollingGroups[LOW_FREQUENCY]["ST_PERCENT"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::ST_PERCENT, 2);
    // Discrete inputs
    m_pollingGroups[LOW_FREQUENCY]["S1"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S1, 1);
    m_pollingGroups[LOW_FREQUENCY]["S2"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S2, 1);
    m_pollingGroups[LOW_FREQUENCY]["S3"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S3, 1);
    m_pollingGroups[LOW_FREQUENCY]["S4"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S4, 1);
    m_pollingGroups[LOW_FREQUENCY]["S5"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S5, 1);
    m_pollingGroups[LOW_FREQUENCY]["S6"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S6, 1);
    m_pollingGroups[LOW_FREQUENCY]["S7"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S7, 1);
    m_pollingGroups[LOW_FREQUENCY]["S8"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S8, 1);
    m_pollingGroups[LOW_FREQUENCY]["S9"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S9, 1);
    m_pollingGroups[LOW_FREQUENCY]["S10"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S10, 1);
    m_pollingGroups[LOW_FREQUENCY]["S11"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S11, 1);
    m_pollingGroups[LOW_FREQUENCY]["S12"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S12, 1);

    // Command outputs
    m_pollingGroups[LOW_FREQUENCY]["K1"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K1, 1);
    m_pollingGroups[LOW_FREQUENCY]["K2"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K2, 1);
    m_pollingGroups[LOW_FREQUENCY]["K3"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K3, 1);
    m_pollingGroups[LOW_FREQUENCY]["K4"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K4, 1);
    m_pollingGroups[LOW_FREQUENCY]["K5"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K5, 1);
    m_pollingGroups[LOW_FREQUENCY]["K6"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K6, 1);

    // Control and status registers
    m_pollingGroups[LOW_FREQUENCY]["M0_STOP_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS, 1);
    m_pollingGroups[LOW_FREQUENCY]["M11_READY_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS, 1);
    m_pollingGroups[LOW_FREQUENCY]["M12_START_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS, 1);
    m_pollingGroups[LOW_FREQUENCY]["M14_COMPLETE_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS, 1);
}

void DeltaModbusClient::pollGroup(PollingGroup group) {
    if (!isConnected()) return;

    auto& registers = m_pollingGroups[group];
    for (auto it = registers.constBegin(); it != registers.constEnd(); ++it) {
        const PolledRegister& reg = it.value();
        m_queue->enqueueRead(reg.type, reg.address, reg.count, it.key());
    }
}


bool DeltaModbusClient::connectToDevice(const QString& address, quint16 port) {
    if (m_client->state() != QModbusDevice::UnconnectedState) {
        disconnectFromDevice();
        QThread::msleep(100);
    }

    m_client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, address);
    m_client->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    if (!m_client->connectDevice()) {
        QString error = "Failed to initiate connection: " + m_client->errorString();
        emit errorOccurred(error);
        return false;
    }

    return true;
}

void DeltaModbusClient::disconnectFromDevice() {
    // Останавливаем все таймеры опроса
    for (QTimer* timer : m_pollTimers) {
        if (timer) {
            timer->stop();
        }
    }

    if (m_handler) {
        m_handler->stop();
    }
    if (m_queue) {
        m_queue->clear();
    }

    if (m_client && m_client->state() != QModbusDevice::UnconnectedState) {
        m_client->disconnectDevice();
    }
}

bool DeltaModbusClient::isConnected() const {
    return m_client->state() == QModbusDevice::ConnectedState;
}

void DeltaModbusClient::setLocalPort(quint16 port) {
    if (m_client->state() != QModbusDevice::UnconnectedState) {
        qWarning() << "Cannot set local port while connected. Disconnecting first.";
        disconnectFromDevice();
    }

    if (auto deltaClient = qobject_cast<CustomModbusClient*>(m_client.data())) {
        deltaClient->bindToPort(port);
        m_localPort = port;
        qDebug() << "Local port set to:" << port;
    } else {
        qCritical() << "Failed to cast to CustomModbusClient";
    }
}

// New unified methods
void DeltaModbusClient::readRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 count) {
    if (!isConnected()) {
        emit errorOccurred("Not connected to device");
        return;
    }
    m_queue->enqueueRead(type, address, count);
}

void DeltaModbusClient::writeRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) {
    if (!isConnected()) {
        QString errorMsg = "Not connected to device";
        emit errorOccurred(errorMsg);
        return;
    }
    m_queue->enqueueWrite(type, address, value);
}

void DeltaModbusClient::removePolledRegister(const QString& name) {
    // Удаляем из всех групп
    for (auto& group : m_pollingGroups) {
        group.remove(name);
    }
}

void DeltaModbusClient::clearPolledRegisters() {
    // Очищаем все группы
    for (auto& group : m_pollingGroups) {
        group.clear();
    }
}

void DeltaModbusClient::addPolledRegisterWithFrequency(const QString& name,
                                                       QModbusDataUnit::RegisterType type,
                                                       quint16 address,
                                                       quint16 count,
                                                       PollingFrequency frequency) {
    PollingGroup group = (frequency == HighFrequency) ? HIGH_FREQUENCY : LOW_FREQUENCY;
    m_pollingGroups[group][name] = PolledRegister(type, address, count);

    // qDebug() << "Added parameter to polling group:" << name
    //          << "address:" << QString::number(address, 16)
    //          << "frequency:" << (frequency == HighFrequency ? "20Hz" : "2Hz");
}

void DeltaModbusClient::setOperationMode(const QString& mode) {
    if (m_currentMode != mode) {
        m_currentMode = mode;
        // adjustPollingForMode(mode);
    }
}

void DeltaModbusClient::onStateChanged(QModbusDevice::State state) {
    switch (state) {
    case QModbusDevice::ConnectedState:
        if (m_handler) {
            m_handler->start();
        }
        // Запускаем все таймеры опроса
        for (QTimer* timer : m_pollTimers) {
            timer->start();
        }
        initializeD0();
        emit connected();
        break;
    case QModbusDevice::UnconnectedState:
        // Останавливаем все таймеры опроса
        for (QTimer* timer : m_pollTimers) {
            timer->stop();
        }
        if (m_handler) {
            m_handler->stop();
        }
        if (m_queue) {
            m_queue->clear();
        }
        emit disconnected();
        break;
    default:
        break;
    }
}

void DeltaModbusClient::onErrorOccurred(QModbusDevice::Error error) {
    if (error != QModbusDevice::NoError) {
        QString errorMsg = m_client->errorString();
        qWarning() << "Modbus error:" << errorMsg;
        emit errorOccurred(errorMsg);

        if (error == QModbusDevice::TimeoutError &&
            m_client->state() != QModbusDevice::UnconnectedState) {
            m_client->disconnectDevice();
        }
    }
}

void DeltaModbusClient::writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, int timeoutMs) {
    if (!isConnected()) {
        emit errorOccurred("Not connected to device");
        return;
    }

    // Сначала записываем значение (оно автоматически попадет в приоритетную очередь)
    writeRegister(type, address, value);

    // Затем добавляем запрос на проверку в ПРИОРИТЕТНУЮ очередь
    VerificationRequest request;
    request.type = type;
    request.address = address;
    request.expectedValue = value;
    request.timestamp = QDateTime::currentDateTime();

    m_verificationRequests[address] = request;

    // Немедленно ставим приоритетное чтение для проверки
    m_queue->enqueuePriorityRead(type, address, 1, "verification");

    // Запускаем таймер проверки
    if (!m_verificationTimer->isActive()) {
        m_verificationTimer->start();
    }

    // Устанавливаем таймаут для проверки
    QTimer::singleShot(timeoutMs, this, [this, address]() {
        if (m_verificationRequests.contains(address)) {
            m_verificationRequests.remove(address);
            emit registerWriteVerified(QModbusDataUnit::HoldingRegisters, address, 0, false);
        }
    });
}

void DeltaModbusClient::onVerificationTimeout() {
    if (m_verificationRequests.isEmpty()) {
        m_verificationTimer->stop();
        return;
    }

    // Для оставшихся запросов на проверку ставим приоритетные чтения
    for (auto it = m_verificationRequests.begin(); it != m_verificationRequests.end(); ++it) {
        quint16 address = it.key();
        m_queue->enqueuePriorityRead(it.value().type, address, 1, "verification");
    }
}

void DeltaModbusClient::onReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, const QString& paramName) {
    emit registerReadCompleted(type, address, value);

    if (!paramName.isEmpty()) {
        emit dataRead(paramName, value);
    }

    // Проверяем, не это ли запрос на верификацию
    if (paramName == "verification" && m_verificationRequests.contains(address)) {
        VerificationRequest request = m_verificationRequests[address];
        bool success = (value == request.expectedValue);
        m_verificationRequests.remove(address);
        emit registerWriteVerified(type, address, value, success);
    }
}

void DeltaModbusClient::onReadsCompleted(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values) {
    emit registersReadCompleted(type, address, values);
}

void DeltaModbusClient::onWriteCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success) {
    emit registerWriteCompleted(type, address, success);
    // Если запись не удалась, удаляем соответствующий запрос на верификацию
    if (!success && m_verificationRequests.contains(address)) {
        m_verificationRequests.remove(address);
        emit registerWriteVerified(type, address, 0, false);
    }
}

void DeltaModbusClient::onRequestFailed(const QString& error) {
    emit errorOccurred(error);
}

void DeltaModbusClient::initializeD0() {
    if (isConnected() && !m_initialD0Written) {
        writeRegister(QModbusDataUnit::HoldingRegisters, 0x0000, 0); // D0 = 0
        m_initialD0Written = true;
    }
}

void DeltaModbusClient::setConnectionParameter(QModbusDevice::ConnectionParameter param, const QVariant& value) {
    if (m_client) {
        m_client->setConnectionParameter(param, value);
    }
}

```

---


## Файл: ModbusClient/src/core/modbus/ModbusRequestHandler.h

```cpp
#pragma once
#include <QObject>
#include <QModbusTcpClient>
#include <QTimer>
#include <QModbusReply>

class IRequestQueue;

class ModbusRequestHandler : public QObject {
    Q_OBJECT
public:
    explicit ModbusRequestHandler(QModbusTcpClient* client,
                                  IRequestQueue* queue,
                                  QObject* parent = nullptr);
    ~ModbusRequestHandler() override;

    void setProcessInterval(int ms);
    void start();
    void stop();
    bool isProcessing() const { return m_processing; }

signals:
    void readCompleted(QModbusDataUnit::RegisterType type, quint16 address,
                      quint16 value, const QString& paramName);
    void readsCompleted(QModbusDataUnit::RegisterType type, quint16 address,
                       const QVector<quint16>& values);
    void writeCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success);
    void requestFailed(const QString& error);

private slots:
    void processNextRequest();
    void handleReadReply();
    void handleWriteReply();

private:
    void sendReadRequest(QModbusDataUnit::RegisterType type, quint16 address,
                        quint16 count, const QString& paramName);
    void sendWriteRequest(QModbusDataUnit::RegisterType type, quint16 address,
                         quint16 value);

    QModbusTcpClient* m_client;
    IRequestQueue* m_queue;
    QTimer* m_processTimer;
    bool m_processing;
    int m_minRequestInterval;
};

```

---


## Файл: ModbusClient/src/gui/mode_selection/ModeSelectionViewController.cpp

```cpp
#include "ModeSelectionViewController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

ModeSelectionViewController::ModeSelectionViewController(ModeController* modeController,
                                                         QObject* parent)
    : QObject(parent)
    , m_modeController(modeController)
    , m_mainWidget(nullptr)
    , m_widgetFactory(nullptr)
    , m_modeButton1(nullptr)
    , m_modeButton2(nullptr)
    , m_modeButton3(nullptr)
    , m_modeButton4(nullptr)
    , m_modeButton5(nullptr)
    , m_stopButton(nullptr)
    , m_connected(false)
    , m_testRunning(false)
{
    setupUI();
    setupConnections();
}

QWidget* ModeSelectionViewController::getWidget() {
    return m_mainWidget;
}

void ModeSelectionViewController::setupUI() {
    m_mainWidget = new QWidget();
    m_widgetFactory = new WidgetFactory(m_mainWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(m_mainWidget);

    // Группа выбора режима
    QGroupBox* modeGroup = new QGroupBox("Выбор режима работы");
    QGridLayout* modeLayout = new QGridLayout(modeGroup);

    // Создаем кнопки выбора режима
    m_modeButton1 = new QPushButton("Расконсервация/\nКонсервация");
    m_modeButton2 = new QPushButton("Холодная прокрутка\nтурбостартера");
    m_modeButton3 = new QPushButton("Регулировка мощности,\nзамер параметров");
    m_modeButton4 = new QPushButton("Холодная прокрутка\nосновного двигателя");
    m_modeButton5 = new QPushButton("Имитация запуска\nосновного двигателя");
    m_stopButton = new QPushButton("СТОП ТЕСТА");

    // Настраиваем размеры кнопок
    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
    for (QPushButton* button : modeButtons) {
        button->setMinimumSize(180, 80);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    m_stopButton->setMinimumSize(180, 80);
    m_stopButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Располагаем кнопки в сетке 2x3
    modeLayout->addWidget(m_modeButton1, 0, 0);
    modeLayout->addWidget(m_modeButton2, 0, 1);
    modeLayout->addWidget(m_modeButton3, 0, 2);
    modeLayout->addWidget(m_modeButton4, 1, 0);
    modeLayout->addWidget(m_modeButton5, 1, 1);
    modeLayout->addWidget(m_stopButton, 1, 2);

    // Устанавливаем растягивание
    modeLayout->setColumnStretch(0, 1);
    modeLayout->setColumnStretch(1, 1);
    modeLayout->setColumnStretch(2, 1);
    modeLayout->setRowStretch(0, 1);
    modeLayout->setRowStretch(1, 1);

    mainLayout->addWidget(modeGroup);
    mainLayout->addStretch();

    // Инициализируем состояние кнопок
    updateButtonStates();
}

void ModeSelectionViewController::setupConnections() {
    if (!m_modeController) {
        qWarning() << "ModeSelectionViewController: ModeController is null!";
        return;
    }

    // Подключаем кнопки НАПРЯМУЮ к ModeController
    connect(m_modeButton1, &QPushButton::clicked, m_modeController, &ModeController::onMode1Clicked);
    connect(m_modeButton2, &QPushButton::clicked, m_modeController, &ModeController::onMode2Clicked);
    connect(m_modeButton3, &QPushButton::clicked, m_modeController, &ModeController::onMode3Clicked);
    connect(m_modeButton4, &QPushButton::clicked, m_modeController, &ModeController::onMode4Clicked);
    connect(m_modeButton5, &QPushButton::clicked, m_modeController, &ModeController::onMode5Clicked);
    connect(m_stopButton, &QPushButton::clicked, m_modeController, &ModeController::onStopClicked);

    // Подключаем сигналы от контроллера режимов для обновления UI
    connect(m_modeController, &ModeController::testStarted, this, &ModeSelectionViewController::onTestStarted);
    connect(m_modeController, &ModeController::testStopped, this, &ModeSelectionViewController::onTestStopped);

    qDebug() << "ModeSelectionViewController: Connections established successfully";
}

void ModeSelectionViewController::updateButtonStates() {
    QString disabledStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #7f8c8d;"
        "border-radius: 8px;"
        "background-color: #bdc3c7;"
        "color: #7f8c8d;"
        "}";

    QString activeModeStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 8px;"
        "background-color: #3498db;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}";

    QString activeStopStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #c0392b;"
        "border-radius: 8px;"
        "background-color: #e74c3c;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "background-color: #a93226;"
        "}";

    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};

    if (!m_connected) {
        // Все кнопки серые и неактивные при отключении
        for (QPushButton* button : modeButtons) {
            if (button) {
                button->setStyleSheet(disabledStyle);
                button->setEnabled(false);
            }
        }
        if (m_stopButton) {
            m_stopButton->setStyleSheet(disabledStyle);
            m_stopButton->setEnabled(false);
        }
    }
    else if (m_testRunning) {
        // Тест запущен: все кнопки режимов неактивны, стоп красная
        for (QPushButton* button : modeButtons) {
            if (button) {
                button->setEnabled(false);
            }
        }
        if (m_stopButton) {
            m_stopButton->setStyleSheet(activeStopStyle);
            m_stopButton->setEnabled(true);
        }
    }
    else {
        // Подключено, тест не запущен: кнопки режимов активные, стоп серая
        for (QPushButton* button : modeButtons) {
            if (button) {
                button->setStyleSheet(activeModeStyle);
                button->setEnabled(true);
            }
        }
        if (m_stopButton) {
            m_stopButton->setStyleSheet(disabledStyle);
            m_stopButton->setEnabled(false);
        }
    }
}

void ModeSelectionViewController::setActiveModeButton(QPushButton* activeButton) {
    QString activeGreenStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #27ae60;"
        "border-radius: 8px;"
        "background-color: #2ecc71;"
        "color: white;"
        "}";

    QString disabledStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #7f8c8d;"
        "border-radius: 8px;"
        "background-color: #bdc3c7;"
        "color: #7f8c8d;"
        "}";

    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
    for (QPushButton* button : modeButtons) {
        if (button && button != activeButton) {
            button->setStyleSheet(disabledStyle);
            button->setEnabled(false);
        } else if (button == activeButton) {
            button->setStyleSheet(activeGreenStyle);
            button->setEnabled(false);
        }
    }
}

void ModeSelectionViewController::setConnectionState(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        updateButtonStates();
    }
}

void ModeSelectionViewController::onTestStarted(const QString& mode) {
    m_testRunning = true;
    m_currentMode = mode;

    qDebug() << "ModeSelectionViewController: Test started:" << mode;

    // Находим активную кнопку и подсвечиваем ее
    QPushButton* activeButton = nullptr;
    if (mode == "Расконсервация/Консервация") activeButton = m_modeButton1;
    else if (mode == "Холодная прокрутка турбостартера") activeButton = m_modeButton2;
    else if (mode == "Регулировка мощности, замер параметров") activeButton = m_modeButton3;
    else if (mode == "Холодная прокрутка основного двигателя") activeButton = m_modeButton4;
    else if (mode == "Имитация запуска основного двигателя") activeButton = m_modeButton5;

    if (activeButton) {
        setActiveModeButton(activeButton);
    }

    updateButtonStates();
}

void ModeSelectionViewController::onTestStopped() {
    m_testRunning = false;
    m_currentMode.clear();
    updateButtonStates();

    qDebug() << "ModeSelectionViewController: Test stopped";
}

void ModeSelectionViewController::onConnectionStateChanged(bool connected) {
    setConnectionState(connected);
}


```

---


## Файл: ModbusClient/src/gui/mode_selection/ModeSelectionViewController.h

```cpp

#pragma once
#include "control/ModeController.h"
#include "../factories/WidgetFactory.h"
#include "core/connection/ConnectionManager.h"
#include <QObject>
#include <QWidget>

class ModeSelectionViewController : public QObject {
    Q_OBJECT
public:
    explicit ModeSelectionViewController(ModeController* modeController,
                                         QObject* parent = nullptr);
    ~ModeSelectionViewController() override = default;

    QWidget* getWidget();

    void setConnectionState(bool connected);

public slots:
    void onTestStarted(const QString& mode);
    void onTestStopped();
    void onConnectionStateChanged(bool connected);

private:
    void setupUI();
    void setupConnections();
    void updateButtonStates();
    void setActiveModeButton(QPushButton* activeButton);

    ModeController* m_modeController;
    QWidget* m_mainWidget;
    WidgetFactory* m_widgetFactory;

    // Кнопки выбора режима
    QPushButton* m_modeButton1;
    QPushButton* m_modeButton2;
    QPushButton* m_modeButton3;
    QPushButton* m_modeButton4;
    QPushButton* m_modeButton5;
    QPushButton* m_stopButton;

    bool m_connected;
    bool m_testRunning;
    QString m_currentMode;
};

```

---


## Файл: ModbusClient/src/gui/widgets/MonitorWidget.h

```cpp
#pragma once
#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;
class QTextEdit;
class DiscreteInputMonitor;
class AnalogValueMonitor;
class DualIndicatorWidget;

class MonitorWidget : public QWidget {
    Q_OBJECT
public:
    explicit MonitorWidget(QWidget* parent = nullptr);

    void setDiscreteMonitor(DiscreteInputMonitor* monitor);
    void setAnalogMonitor(AnalogValueMonitor* monitor);

    // Новые методы для получения элементов управления (2 кнопки вместо 4)
    QPushButton* actionButton1() const { return m_actionButton1; }
    QPushButton* actionButton2() const { return m_actionButton2; }
    QTextEdit* logTextEdit() const { return m_logTextEdit; }

private:
    void setupUI();
    QWidget* createDiscreteInputsWidget();
    QWidget* createCommandOutputsWidget();
    QWidget* createAnalogValuesWidget();
    QWidget* createControlButtonsWidget(); // Теперь создает 2 кнопки
    QWidget* createMonitorLogWidget();

    QVector<QLabel*> m_discreteLabels;
    QVector<QLabel*> m_commandLabels;

    DualIndicatorWidget* m_adIndicator;
    DualIndicatorWidget* m_tkIndicator;
    DualIndicatorWidget* m_stIndicator;

    // Элементы управления (2 кнопки вместо 4)
    QPushButton* m_actionButton1; // Для: Проверка готовности, Пуск, Повторение запуска
    QPushButton* m_actionButton2; // Для: Выход, Прерывание, Стоп
    QTextEdit* m_logTextEdit;
};

```

---


## Файл: ModbusClient/src/gui/widgets/DualIndicatorWidget.h

```cpp
#pragma once
#include <QWidget>
#include "SpeedometerWidget.h"
#include "core/interfaces/IIndicator.h"

class QLabel;
class QVBoxLayout;

/**
 * @brief Композитный виджет, объединяющий спидометр и LCD индикатор
 * Реализует принцип SRP - отвечает только за компоновку двух индикаторов
 */
class DualIndicatorWidget : public QWidget, public IDualIndicator {
    Q_OBJECT
public:
    explicit DualIndicatorWidget(const QString& primaryTitle,
                               const QString& secondaryTitle,
                               double primaryMin, double primaryMax,
                               double secondaryMin, double secondaryMax,
                               QWidget* parent = nullptr);

    // Методы для управления спидометром
    void setValue(double value);
    void setRange(double min, double max);
    void setColor(const QColor& color);
    void setAlertLevel(double level, const QColor& color = Qt::red);

    // Методы для управления LCD индикатором
    void setSecondaryValue(double value);
    void setSecondaryRange(double min, double max);
    void setSecondaryVisible(bool visible);

    // Получение внутренних компонентов (для обратной совместимости)
    SpeedometerWidget* speedometer() const { return m_speedometer; }
    QLabel* lcdLabel() const { return m_lcdLabel; }

private:
    void updateLcdDisplay();

    SpeedometerWidget* m_speedometer;
    QLabel* m_lcdLabel;
    QString m_secondaryTitle;
    double m_secondaryValue;
    double m_secondaryMin;
    double m_secondaryMax;
};

```

---


## Файл: ModbusClient/src/gui/widgets/ConnectionWidget.cpp

```cpp
#include "ConnectionWidget.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>

ConnectionWidget::ConnectionWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

void ConnectionWidget::setupUI() {
    QFormLayout* layout = new QFormLayout(this);

    m_addressEdit = new QLineEdit("192.168.37.169");
    m_serverPortSpinBox = new QSpinBox();
    m_serverPortSpinBox->setRange(1, 65535);
    m_serverPortSpinBox->setValue(502);

    m_clientPortSpinBox = new QSpinBox();
    m_clientPortSpinBox->setRange(1024, 65535);
    m_clientPortSpinBox->setValue(3201);

    m_connectButton = new QPushButton("Подключить");
    m_disconnectButton = new QPushButton("Отключить");
    m_disconnectButton->setEnabled(false);

    m_statusLabel = new QLabel("Отключен");
    m_statusLabel->setStyleSheet("color: red; font-weight: bold;");

    layout->addRow("IP адрес:", m_addressEdit);
    layout->addRow("Порт сервера:", m_serverPortSpinBox);
    layout->addRow("Порт клиента:", m_clientPortSpinBox);
    layout->addRow(m_connectButton);
    layout->addRow(m_disconnectButton);
    layout->addRow("Статус:", m_statusLabel);

    connect(m_connectButton, &QPushButton::clicked, this, [this]() {
        emit connectRequested(address(), serverPort(), clientPort());
    });
    connect(m_disconnectButton, &QPushButton::clicked,
            this, &ConnectionWidget::disconnectRequested);    
}

QString ConnectionWidget::address() const {
    return m_addressEdit->text();
}

quint16 ConnectionWidget::serverPort() const {
    return static_cast<quint16>(m_serverPortSpinBox->value());
}

quint16 ConnectionWidget::clientPort() const {
    return static_cast<quint16>(m_clientPortSpinBox->value());
}

void ConnectionWidget::setConnected(bool connected) {
    m_connectButton->setEnabled(!connected);
    m_disconnectButton->setEnabled(connected);
    m_addressEdit->setEnabled(!connected);
    m_serverPortSpinBox->setEnabled(!connected);
    m_clientPortSpinBox->setEnabled(!connected);
}

void ConnectionWidget::setStatus(const QString& status) {
    m_statusLabel->setText(status);
    bool connected = (status == "ПОДКЛЮЧЕН");
    m_statusLabel->setStyleSheet(connected ?
                                     "color: green; font-weight: bold;" :
                                     "color: red; font-weight: bold;");
}


```

---


## Файл: ModbusClient/src/gui/widgets/DualIndicatorWidget.cpp

```cpp
#include "DualIndicatorWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>

DualIndicatorWidget::DualIndicatorWidget(const QString& primaryTitle,
                                       const QString& secondaryTitle,
                                       double primaryMin, double primaryMax,
                                       double secondaryMin, double secondaryMax,
                                       QWidget* parent)
    : QWidget(parent)
    , m_speedometer(new SpeedometerWidget(primaryTitle, primaryMin, primaryMax, this))
    , m_lcdLabel(new QLabel(this))
    , m_secondaryTitle(secondaryTitle)
    , m_secondaryValue(0.0)
    , m_secondaryMin(secondaryMin)
    , m_secondaryMax(secondaryMax)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(2, 2, 2, 2);

    layout->addWidget(m_speedometer);
    layout->addWidget(m_lcdLabel);

    // Настройка LCD стиля
    m_lcdLabel->setAlignment(Qt::AlignCenter);
    m_lcdLabel->setMinimumHeight(25);
    m_lcdLabel->setStyleSheet(
        "QLabel {"
        "background-color: #f0f0f0;"
        "border: 2px solid #c0c0c0;"
        "border-radius: 3px;"
        "padding: 2px 4px;"
        "font-weight: bold;"
        "font-family: 'Courier New', monospace;"
        "color: #000000;"
        "}"
    );

    // Устанавливаем шрифт фиксированной ширины для LCD эффекта
    QFont lcdFont("Courier New", 10, QFont::Bold);
    m_lcdLabel->setFont(lcdFont);

    updateLcdDisplay();
}

void DualIndicatorWidget::setValue(double value) {
    m_speedometer->setValue(value);
}

void DualIndicatorWidget::setRange(double min, double max) {
    m_speedometer->setRange(min, max);
}

void DualIndicatorWidget::setColor(const QColor& color) {
    m_speedometer->setColor(color);
}

void DualIndicatorWidget::setAlertLevel(double level, const QColor& color) {
    m_speedometer->setAlertLevel(level, color);
}

void DualIndicatorWidget::setSecondaryValue(double value) {
    m_secondaryValue = qBound(m_secondaryMin, value, m_secondaryMax);
    updateLcdDisplay();
}

void DualIndicatorWidget::setSecondaryRange(double min, double max) {
    m_secondaryMin = min;
    m_secondaryMax = max;
    // Обновляем текущее значение в рамках нового диапазона
    m_secondaryValue = qBound(min, m_secondaryValue, max);
    updateLcdDisplay();
}

void DualIndicatorWidget::setSecondaryVisible(bool visible) {
    m_lcdLabel->setVisible(visible);
}

void DualIndicatorWidget::updateLcdDisplay() {
    QString valueText = QString("%1").arg(m_secondaryValue, 0, 'f', 0);
    QString text = QString("%1\n%2")
                   .arg(valueText)
                   .arg(m_secondaryTitle);
    m_lcdLabel->setText(text);

    // Опционально: меняем цвет если значение превышает 100%
    if (m_speedometer && m_speedometer->value() > 100.0) {
        m_lcdLabel->setStyleSheet(
            "QLabel {"
            "background-color: #fff0f0;"
            "border: 2px solid #ff0000;"
            "border-radius: 3px;"
            "padding: 2px 4px;"
            "font-weight: bold;"
            "font-family: 'Courier New', monospace;"
            "color: #ff0000;"
            "}"
        );
    } else {
        m_lcdLabel->setStyleSheet(
            "QLabel {"
            "background-color: #f0f0f0;"
            "border: 2px solid #c0c0c0;"
            "border-radius: 3px;"
            "padding: 2px 4px;"
            "font-weight: bold;"
            "font-family: 'Courier New', monospace;"
            "color: #000000;"
            "}"
        );
    }
}

```

---


## Файл: ModbusClient/src/gui/widgets/SpeedometerWidget.cpp

```cpp
#include "SpeedometerWidget.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

#ifndef M_PI
const double M_PI = 3.14159265358979323846;
#endif

SpeedometerWidget::SpeedometerWidget(const QString& title, double minValue, double maxValue, QWidget* parent)
    : QWidget(parent)
    , m_title(title)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_currentValue(minValue)
    , m_color(Qt::blue)
    , m_alertLevel(maxValue)
    , m_alertColor(Qt::red)
    , m_alertActive(false)
{
    setMinimumSize(200, 200);
}

void SpeedometerWidget::setValue(const double value) {
    m_currentValue = qBound(m_minValue, value, m_maxValue);
    m_alertActive = (m_currentValue >= m_alertLevel);
    update();
}

void SpeedometerWidget::setRange(const double min, const double max){
    m_minValue=min;
    m_maxValue=max;
    update();
}


void SpeedometerWidget::setColor(const QColor& color) {
    m_color = color;
    update();
}

void SpeedometerWidget::setAlertLevel(double level, const QColor& color) {
    m_alertLevel = level;
    m_alertColor = color;
    update();
}

void SpeedometerWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), Qt::white);

    // Рисуем компоненты спидометра
    drawBackground(painter);
    drawTicks(painter);
    drawLabels(painter);
    drawNeedle(painter);
    drawValue(painter);
}

void SpeedometerWidget::drawBackground(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);

    // Градиент для фона спидометра
    QConicalGradient gradient(rect.center(), -50);
    gradient.setColorAt(0, QColorConstants::Svg::lightsalmon);
    gradient.setColorAt(0.5, Qt::yellow);
    gradient.setColorAt(1, Qt::yellow);

    painter.setBrush(gradient);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(rect);
}

void SpeedometerWidget::drawTicks(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);
    QPointF center = rect.center();
    double radius = size / 3;

    painter.setPen(QPen(Qt::black, 2));

    // Основные деления
    for (int i = 0; i <= 11; ++i) {
        double angle = 225 - i * 270.0 / 11.0; // 225° до -45°
        double rad = angle * M_PI / 180.0;

        double innerRadius = radius * 0.7;
        double outerRadius = radius * 0.9;

        QPointF innerPoint(center.x() + innerRadius * cos(rad),
                           center.y() - innerRadius * sin(rad));
        QPointF outerPoint(center.x() + outerRadius * cos(rad),
                           center.y() - outerRadius * sin(rad));

        painter.drawLine(innerPoint, outerPoint);
    }

    // Мелкие деления
    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i <= 55; ++i) {
        double angle = 225 - i * 270.0 / 55.0;
        double rad = angle * M_PI / 180.0;

        double innerRadius = radius * 0.8;
        double outerRadius = radius * 0.85;

        QPointF innerPoint(center.x() + innerRadius * cos(rad),
                           center.y() - innerRadius * sin(rad));
        QPointF outerPoint(center.x() + outerRadius * cos(rad),
                           center.y() - outerRadius * sin(rad));

        painter.drawLine(innerPoint, outerPoint);
    }
}

void SpeedometerWidget::drawLabels(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);
    QPointF center = rect.center();
    double radius = size / 2;

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    for (int i = 0; i <= 11; ++i) {
        double value = m_minValue + i * (m_maxValue - m_minValue) / 11.0;
        double angle = 225 - i * 270.0 / 11.0;
        double rad = angle * M_PI / 180.0;

        double labelRadius = radius * 0.75;
        QPointF labelPoint(center.x() + labelRadius * cos(rad),
                           center.y() - labelRadius * sin(rad));

        QString label = QString::number(value, 'f', 0);
        QRectF textRect(labelPoint.x() - 20, labelPoint.y() - 10, 40, 20);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }

    // Заголовок
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRectF(0, center.y() + radius * 0.4, width(), 30),
                     Qt::AlignCenter, m_title);
}

void SpeedometerWidget::drawNeedle(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);
    QPointF center = rect.center();
    double radius = size / 2;

    // Вычисляем угол стрелки
    double normalizedValue = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
    double angle = 225 - normalizedValue * 270.0; // 225° до -45°
    double rad = angle * M_PI / 180.0;

    // Рисуем стрелку
    QPen needlePen(m_alertActive ? m_alertColor : m_color, 4);
    painter.setPen(needlePen);

    double needleLength = radius * 0.8;
    QPointF needleEnd(center.x() + needleLength * cos(rad),
                      center.y() - needleLength * sin(rad));

    painter.drawLine(center, needleEnd);

    // Центральная точка
    painter.setBrush(m_alertActive ? m_alertColor : m_color);
    painter.drawEllipse(center, 5, 5);
}

void SpeedometerWidget::drawValue(QPainter& painter) {
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    painter.setPen(m_alertActive ? m_alertColor : m_color);

    QString valueText = QString("%1").arg(m_currentValue, 0, 'f', 0);
    QRectF valueRect(0, height() * 0.6, width(), 10);
    painter.drawText(valueRect, Qt::AlignCenter, valueText);
}

```

---


## Файл: ModbusClient/src/gui/widgets/MonitorWidget.cpp

```cpp
#include "MonitorWidget.h"
#include "monitoring/DiscreteInputMonitor.h"
#include "monitoring/AnalogValueMonitor.h"
#include "DualIndicatorWidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTextEdit>

MonitorWidget::MonitorWidget(QWidget* parent)
    : QWidget(parent)
    , m_adIndicator(nullptr)
    , m_tkIndicator(nullptr)
    , m_stIndicator(nullptr)
    , m_actionButton1(nullptr)
    , m_actionButton2(nullptr)
    , m_logTextEdit(nullptr)
{
    setupUI();
}

void MonitorWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Верхняя часть - дискретные сигналы в двух колонках
    QHBoxLayout* discreteLayout = new QHBoxLayout();
    discreteLayout->addWidget(createDiscreteInputsWidget());
    discreteLayout->addWidget(createCommandOutputsWidget());
    layout->addLayout(discreteLayout);

    // Аналоговые значения со спидометрами
    layout->addWidget(createAnalogValuesWidget());

    // Кнопки управления (теперь 2 кнопки)
    layout->addWidget(createControlButtonsWidget());

    // Журнал событий мониторинга
    layout->addWidget(createMonitorLogWidget());

    layout->addStretch();
}

QWidget* MonitorWidget::createDiscreteInputsWidget() {
    QGroupBox* group = new QGroupBox("Дискретные входы (S1-S12)");
    QGridLayout* layout = new QGridLayout(group);
    layout->setVerticalSpacing(5);
    layout->setHorizontalSpacing(10);

    QStringList labels = {
        "S1: Откл БУТС", "S2: ОРТС", "S3: ЭМЗС", "S4: АЗТС",
        "S5: ЭСТС", "S6: Откл ЭСТС (СОЭС)", "S7: ИП АЗТС Вкл.", "S8: ИП ЭСТС Вкл.",
        "S9: 1й канал АЗТС Вкл.", "S10: 2й канал АЗТС Вкл.", "S11: ЭСТС Вкл.", "S12: ПЧ готов"
    };

    // Распределяем по 6 элементов в каждом столбце
    for (int i = 0; i < labels.size(); ++i) {
        int row = i % 6;
        int column = i / 6;

        QLabel* nameLabel = new QLabel(labels[i]);
        nameLabel->setMinimumWidth(180);
        nameLabel->setStyleSheet("background-color: grey; color: white; padding: 6px; border-radius: 3px;");
        nameLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(nameLabel, row, column);
        m_discreteLabels.append(nameLabel);
    }

    return group;
}

QWidget* MonitorWidget::createCommandOutputsWidget() {
    QGroupBox* group = new QGroupBox("Командные выходы (K1-K6)");
    QGridLayout* layout = new QGridLayout(group);

    QStringList labels = {
        "K1: Пуск ТС", "K2: Стоп ТС", "K3: СТОП-кран",
        "K4: Режим Консервации", "K5: Режим Холодной прокрутки", "K6: Активация Выходов ПЧ"
    };

    for (int i = 0; i < labels.size(); ++i) {
        QLabel* nameLabel = new QLabel(labels[i]);
        nameLabel->setMinimumWidth(200);
        nameLabel->setStyleSheet("background-color: grey; color: white; padding: 8px; border-radius: 4px;");
        nameLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(nameLabel, i, 0);
        m_commandLabels.append(nameLabel);
    }

    return group;
}

QWidget* MonitorWidget::createAnalogValuesWidget() {
    QGroupBox* group = new QGroupBox("Аналоговые значения");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем композитные индикаторы
    m_adIndicator = new DualIndicatorWidget("АД %", "об/мин", 0, 110, 0, 4996);
    m_tkIndicator = new DualIndicatorWidget("ТК %", "об/мин", 0, 110, 0, 71500);
    m_stIndicator = new DualIndicatorWidget("СТ %", "об/мин", 0, 110, 0, 64750);

    // Настройка цветов
    m_adIndicator->setColor(Qt::blue);
    m_tkIndicator->setColor(Qt::red);
    m_stIndicator->setColor(Qt::green);

    // Установка уровней предупреждения
    m_adIndicator->setAlertLevel(100, Qt::red);
    m_tkIndicator->setAlertLevel(100, Qt::red);
    m_stIndicator->setAlertLevel(100, Qt::red);

    layout->addWidget(m_adIndicator);
    layout->addWidget(m_tkIndicator);
    layout->addWidget(m_stIndicator);
    layout->addStretch();

    return group;
}

void MonitorWidget::setDiscreteMonitor(DiscreteInputMonitor* monitor) {
    if (monitor) {
        monitor->setDiscreteLabels(m_discreteLabels);
        monitor->setCommandLabels(m_commandLabels);
    }
}

void MonitorWidget::setAnalogMonitor(AnalogValueMonitor* monitor) {
    if (monitor) {
        monitor->setIndicators(m_adIndicator, m_tkIndicator, m_stIndicator);
    }
}

QWidget* MonitorWidget::createControlButtonsWidget() {
    QGroupBox* group = new QGroupBox("Управление запуском");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем только 2 кнопки управления
    m_actionButton1 = new QPushButton("Проверка готовности");
    m_actionButton2 = new QPushButton("Выход");

    // Настраиваем размеры кнопок
    m_actionButton1->setMinimumHeight(40);
    m_actionButton2->setMinimumHeight(40);

    // Настраиваем стили
    QString buttonStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 12px;"
        "padding: 8px 12px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 5px;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "background-color: #bdc3c7;"
        "border-color: #95a5a6;"
        "color: #7f8c8d;"
        "}";

    // Начальные стили для состояния 1
    m_actionButton1->setStyleSheet(buttonStyle + "background-color: #3498db;");
    m_actionButton2->setStyleSheet(buttonStyle + "background-color: #e74c3c;");

    // Начальное состояние кнопок
    m_actionButton1->setEnabled(true);
    m_actionButton2->setEnabled(true);

    // Добавляем кнопки в layout
    layout->addWidget(m_actionButton1);
    layout->addWidget(m_actionButton2);

    return group;
}

QWidget* MonitorWidget::createMonitorLogWidget() {
    QGroupBox* group = new QGroupBox("Журнал событий мониторинга");
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setPlaceholderText("Журнал событий мониторинга...");

    layout->addWidget(m_logTextEdit);
    return group;
}

```

---


## Файл: ModbusClient/src/gui/widgets/SpeedometerWidget.h

```cpp
#pragma once
#include <QWidget>
#include <QPainter>
#include <QConicalGradient>
#include <QTimer>

class SpeedometerWidget : public QWidget {
    Q_OBJECT
public:
    explicit SpeedometerWidget(const QString& title, double minValue, double maxValue,
                               QWidget* parent = nullptr);

    void setValue(const double value);
    inline double value() const {return m_currentValue;}
    void setRange(const double min, const double max);
    void setColor(const QColor& color);
    void setAlertLevel(double level, const QColor& color = Qt::red);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawBackground(QPainter& painter);
    void drawTicks(QPainter& painter);
    void drawLabels(QPainter& painter);
    void drawNeedle(QPainter& painter);
    void drawValue(QPainter& painter);

    QString m_title;
    double m_minValue;
    double m_maxValue;
    double m_currentValue;
    QColor m_color;
    double m_alertLevel;
    QColor m_alertColor;
    bool m_alertActive;
};

```

---


## Файл: ModbusClient/src/gui/widgets/ConnectionWidget.h

```cpp
#pragma once
#include <QWidget>

class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;

class ConnectionWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionWidget(QWidget* parent = nullptr);

    QString address() const;
    quint16 serverPort() const;
    quint16 clientPort() const;

    void setConnected(bool connected);
    void setStatus(const QString& status);

signals:
    void connectRequested(const QString& address, quint16 serverPort, quint16 clientPort);
    void disconnectRequested();

private:
    void setupUI();

    QLineEdit* m_addressEdit;
    QSpinBox* m_serverPortSpinBox;
    QSpinBox* m_clientPortSpinBox;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;
    QLabel* m_statusLabel;
};


```

---


## Файл: ModbusClient/src/gui/widgets/ChartWidget.cpp

```cpp
#include "ChartWidget.h"
#include "data/interfaces/IDataRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGridLayout>
#include <limits>

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_repository(nullptr)
    , m_timeRange(300)
    , m_recording(false)
    // , m_mainSplitter(new QSplitter(Qt::Horizontal, this))
    // , m_speedometerPanel(new QWidget())
    // , m_speedometerAD(new SpeedometerWidget("АД", 0, 3000))
    // , m_speedometerTK(new SpeedometerWidget("ТК", 0, 15000))
    // , m_speedometerST(new SpeedometerWidget("СТ", 0, 20000))
    , m_chartPanel(new QWidget())
    , m_chart(new QChart())
    , m_chartView(new QChartView(m_chart))
    , m_seriesAD(new QLineSeries())
    , m_seriesTK(new QLineSeries())
    , m_seriesST(new QLineSeries())
    , m_axisX(new QDateTimeAxis())
    , m_axisY_AD(new QValueAxis())
    , m_axisY_TK(new QValueAxis())
    , m_axisY_ST(new QValueAxis())
    , m_autoScaleAD(new QCheckBox("Авто"))
    , m_autoScaleTK(new QCheckBox("Авто"))
    , m_autoScaleST(new QCheckBox("Авто"))
    , m_minAD(new QDoubleSpinBox())
    , m_maxAD(new QDoubleSpinBox())
    , m_minTK(new QDoubleSpinBox())
    , m_maxTK(new QDoubleSpinBox())
    , m_minST(new QDoubleSpinBox())
    , m_maxST(new QDoubleSpinBox())
    , m_timeRangeSpin(new QSpinBox())
    , m_showGrid(new QCheckBox("Показать сетку"))
    , m_resetZoomButton(new QPushButton("Сброс zoom"))
{
    setupChart();
    setupMultiAxis();
    setupControlPanel();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Настройка разделителя
    // m_mainSplitter->addWidget(m_speedometerPanel);
    // m_mainSplitter->addWidget(m_chartPanel);
    // m_mainSplitter->setSizes({400, 800}); // 400px спидометры, 800px графики

    mainLayout->addWidget(m_chartPanel);
    setLayout(mainLayout);

    // Подключение сигналов
    connect(m_autoScaleAD, &QCheckBox::stateChanged, this, &ChartWidget::onAutoScaleADChanged);
    connect(m_autoScaleTK, &QCheckBox::stateChanged, this, &ChartWidget::onAutoScaleTKChanged);
    connect(m_autoScaleST, &QCheckBox::stateChanged, this, &ChartWidget::onAutoScaleSTChanged);
    connect(m_minAD, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleADChanged);
    connect(m_maxAD, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleADChanged);
    connect(m_minTK, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleTKChanged);
    connect(m_maxTK, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleTKChanged);
    connect(m_minST, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleSTChanged);
    connect(m_maxST, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleSTChanged);
    connect(m_timeRangeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChartWidget::onTimeRangeChanged);
    connect(m_showGrid, &QCheckBox::toggled, this, &ChartWidget::onGridToggled);
    connect(m_resetZoomButton, &QPushButton::clicked, this, &ChartWidget::resetZoom);
}

void ChartWidget::setupControlPanel() {
    // Настройка правой панели с графиками
    QVBoxLayout* chartLayout = new QVBoxLayout(m_chartPanel);
    chartLayout->setContentsMargins(0, 0, 0, 0);

    // Заголовок панели графиков
    QLabel* chartTitle = new QLabel("ГРАФИКИ ОБОРОТОВ ВО ВРЕМЕНИ");
    chartTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont = chartTitle->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    chartTitle->setFont(titleFont);
    chartLayout->addWidget(chartTitle);

    chartLayout->addWidget(m_chartView);

    // Панель управления масштабом и отображением
    QGroupBox* controlGroup = new QGroupBox("Управление масштабом и отображением");
    QGridLayout* controlLayout = new QGridLayout(controlGroup);

    // Масштаб по времени
    QGroupBox* timeGroup = new QGroupBox("Время (сек)");
    QVBoxLayout* timeLayout = new QVBoxLayout(timeGroup);
    m_timeRangeSpin->setRange(10, 3600);
    m_timeRangeSpin->setValue(m_timeRange);
    m_timeRangeSpin->setSuffix(" сек");
    timeLayout->addWidget(m_timeRangeSpin);
    controlLayout->addWidget(timeGroup, 0, 0);

    // Масштаб АД
    QGroupBox* adGroup = new QGroupBox("АД об/мин");
    QFormLayout* adLayout = new QFormLayout(adGroup);
    adLayout->addRow("Авто:", m_autoScaleAD);
    adLayout->addRow("Мин:", m_minAD);
    adLayout->addRow("Макс:", m_maxAD);
    controlLayout->addWidget(adGroup, 0, 1);

    // Масштаб ТК
    QGroupBox* tkGroup = new QGroupBox("ТК об/мин");
    QFormLayout* tkLayout = new QFormLayout(tkGroup);
    tkLayout->addRow("Авто:", m_autoScaleTK);
    tkLayout->addRow("Мин:", m_minTK);
    tkLayout->addRow("Макс:", m_maxTK);
    controlLayout->addWidget(tkGroup, 0, 2);

    // Масштаб СТ
    QGroupBox* stGroup = new QGroupBox("СТ об/мин");
    QFormLayout* stLayout = new QFormLayout(stGroup);
    stLayout->addRow("Авто:", m_autoScaleST);
    stLayout->addRow("Мин:", m_minST);
    stLayout->addRow("Макс:", m_maxST);
    controlLayout->addWidget(stGroup, 0, 3);

    // Управление отображением
    QGroupBox* displayGroup = new QGroupBox("Отображение");
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    displayLayout->addWidget(m_showGrid);
    displayLayout->addWidget(m_resetZoomButton);
    controlLayout->addWidget(displayGroup, 0, 4);

    chartLayout->addWidget(controlGroup);

    // Настройка спинбоксов
    setupControlPanelValues();
}

void ChartWidget::setupControlPanelValues() {
    // Настройка спинбоксов для АД
    m_minAD->setRange(-100000, 100000);
    m_maxAD->setRange(-100000, 100000);
    m_minAD->setValue(0);
    m_maxAD->setValue(3000);
    m_minAD->setSingleStep(100);
    m_maxAD->setSingleStep(100);

    // Настройка спинбоксов для ТК
    m_minTK->setRange(-100000, 100000);
    m_maxTK->setRange(-100000, 100000);
    m_minTK->setValue(0);
    m_maxTK->setValue(71500);
    m_minTK->setSingleStep(1500);
    m_maxTK->setSingleStep(1500);

    // Настройка спинбоксов для СТ
    m_minST->setRange(-100000, 100000);
    m_maxST->setRange(-100000, 100000);
    m_minST->setValue(0);
    m_maxST->setValue(65000);
    m_minST->setSingleStep(1500);
    m_maxST->setSingleStep(1500);

    // По умолчанию авто масштабирование
    m_autoScaleAD->setChecked(true);
    m_autoScaleTK->setChecked(true);
    m_autoScaleST->setChecked(true);

    // Сетка по умолчанию выключена
    m_showGrid->setChecked(false);
}

void ChartWidget::setupChart() {
    // Настройка серий
    m_seriesAD->setName("АД об/мин");
    m_seriesAD->setColor(Qt::blue);

    m_seriesTK->setName("ТК об/мин");
    m_seriesTK->setColor(Qt::red);

    m_seriesST->setName("СТ об/мин");
    m_seriesST->setColor(Qt::green);

    m_chart->addSeries(m_seriesAD);
    m_chart->addSeries(m_seriesTK);
    m_chart->addSeries(m_seriesST);

    m_chart->setTitle("Мониторинг оборотов во времени");
    m_chart->legend()->setVisible(true);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    // Включаем zoom и скроллинг
    m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    m_chartView->setInteractive(true);
}

void ChartWidget::setupMultiAxis() {
    // Ось X
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("Время");
    m_axisX->setGridLineVisible(false);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    // Ось Y для АД (синяя)
    m_axisY_AD->setTitleText("АД об/мин");
    m_axisY_AD->setLinePenColor(Qt::blue);
    m_axisY_AD->setLabelsColor(Qt::blue);
    m_axisY_AD->setTitleBrush(QBrush(Qt::blue));
    m_axisY_AD->setGridLineVisible(false);
    m_chart->addAxis(m_axisY_AD, Qt::AlignLeft);
    m_seriesAD->attachAxis(m_axisY_AD);

    // Ось Y для ТК (красная) - справа
    m_axisY_TK->setTitleText("ТК об/мин");
    m_axisY_TK->setLinePenColor(Qt::red);
    m_axisY_TK->setLabelsColor(Qt::red);
    m_axisY_TK->setTitleBrush(QBrush(Qt::red));
    m_axisY_TK->setGridLineVisible(false);
    m_chart->addAxis(m_axisY_TK, Qt::AlignRight);
    m_seriesTK->attachAxis(m_axisY_TK);

    // Ось Y для СТ (зеленая) - справа со смещением
    m_axisY_ST->setTitleText("СТ об/мин");
    m_axisY_ST->setLinePenColor(Qt::green);
    m_axisY_ST->setLabelsColor(Qt::green);
    m_axisY_ST->setTitleBrush(QBrush(Qt::green));
    m_axisY_ST->setGridLineVisible(false);
    m_chart->addAxis(m_axisY_ST, Qt::AlignRight);
    m_seriesST->attachAxis(m_axisY_ST);

    // Все серии прикрепляем к оси X
    m_seriesAD->attachAxis(m_axisX);
    m_seriesTK->attachAxis(m_axisX);
    m_seriesST->attachAxis(m_axisX);
}

void ChartWidget::updateGrid() {
    bool showGrid = m_showGrid->isChecked();

    // Включаем/выключаем основную сетку графика
    m_chart->setBackgroundVisible(showGrid);

    // Настраиваем сетку для осей
    QPen gridPen(QColor(200, 200, 200, 100)); // Светло-серый с прозрачностью
    gridPen.setStyle(Qt::DashLine);

    // Ось X
    m_axisX->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisX->setGridLinePen(gridPen);
        m_axisX->setShadesVisible(true);
        m_axisX->setShadesBrush(QBrush(QColor(240, 240, 240, 50)));
    }

    // Ось Y АД
    m_axisY_AD->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisY_AD->setGridLinePen(gridPen);
    }

    // Ось Y ТК
    m_axisY_TK->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisY_TK->setGridLinePen(gridPen);
    }

    // Ось Y СТ
    m_axisY_ST->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisY_ST->setGridLinePen(gridPen);
    }
}

// Слоты для управления масштабом
void ChartWidget::onAutoScaleADChanged(int state) {
    m_minAD->setEnabled(state == Qt::Unchecked);
    m_maxAD->setEnabled(state == Qt::Unchecked);
    if (state == Qt::Checked) {
        updateChart(); // Пересчитать авто масштаб
    }
}

void ChartWidget::onAutoScaleTKChanged(int state) {
    m_minTK->setEnabled(state == Qt::Unchecked);
    m_maxTK->setEnabled(state == Qt::Unchecked);
    if (state == Qt::Checked) {
        updateChart();
    }
}

void ChartWidget::onAutoScaleSTChanged(int state) {
    m_minST->setEnabled(state == Qt::Unchecked);
    m_maxST->setEnabled(state == Qt::Unchecked);
    if (state == Qt::Checked) {
        updateChart();
    }
}

void ChartWidget::onManualScaleADChanged() {
    if (!m_autoScaleAD->isChecked()) {
        m_axisY_AD->setRange(m_minAD->value(), m_maxAD->value());
    }
}

void ChartWidget::onManualScaleTKChanged() {
    if (!m_autoScaleTK->isChecked()) {
        m_axisY_TK->setRange(m_minTK->value(), m_maxTK->value());
    }
}

void ChartWidget::onManualScaleSTChanged() {
    if (!m_autoScaleST->isChecked()) {
        m_axisY_ST->setRange(m_minST->value(), m_maxST->value());
    }
}

void ChartWidget::onTimeRangeChanged() {
    m_timeRange = m_timeRangeSpin->value();
    updateChart();
}

void ChartWidget::onGridToggled(bool enabled) {
    updateGrid();
}

void ChartWidget::resetZoom() {
    // Сбрасываем zoom к исходному масштабу
    m_chart->zoomReset();

    // Восстанавливаем авто масштабирование если нужно
    if (m_autoScaleAD->isChecked() || m_autoScaleTK->isChecked() || m_autoScaleST->isChecked()) {
        updateChart();
    }
}

void ChartWidget::applyManualScale() {
    if (!m_autoScaleAD->isChecked()) {
        m_axisY_AD->setRange(m_minAD->value(), m_maxAD->value());
    }
    if (!m_autoScaleTK->isChecked()) {
        m_axisY_TK->setRange(m_minTK->value(), m_maxTK->value());
    }
    if (!m_autoScaleST->isChecked()) {
        m_axisY_ST->setRange(m_minST->value(), m_maxST->value());
    }
}

void ChartWidget::startTestRecording() {
    m_recording = true;
    m_chart->setTitle("Мониторинг оборотов [ЗАПИСЬ]");

    // ОЧИЩАЕМ ДАННЫЕ РЕПОЗИТОРИЯ ПРИ НАЧАЛЕ ТЕСТА
    if (m_repository) {
        m_repository->clearData("AD_RPM");
        m_repository->clearData("TK_RPM");
        m_repository->clearData("ST_RPM");
        m_repository->clearData("TK_PERCENT");
        m_repository->clearData("ST_PERCENT");
    }

    // Очищаем предыдущие данные
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();

    // Сбрасываем масштаб если авто
    if (m_autoScaleAD->isChecked() || m_autoScaleTK->isChecked() || m_autoScaleST->isChecked()) {
        updateChart();
    }
}

void ChartWidget::stopTestRecording() {
    m_recording = false;
    m_chart->setTitle("Мониторинг оборотов [ОСТАНОВЛЕНО]");
}

void ChartWidget::clearChart() {
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();
    updateChart();
}

void ChartWidget::loadHistoricalData(const QVector<DataPointRecord>& points) {
    // Очищаем текущие данные
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();

    // Группируем точки по параметрам
    QVector<DataPointRecord> adPoints, tkPoints, stPoints;

    for (const auto& point : points) {
        if (point.parameter == "AD_RPM") {
            adPoints.append(point);
        } else if (point.parameter == "TK_RPM") {
            tkPoints.append(point);
        } else if (point.parameter == "ST_RPM") {
            stPoints.append(point);
        }
    }

    // Добавляем точки в серии
    for (const auto& point : adPoints) {
        m_seriesAD->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }
    for (const auto& point : tkPoints) {
        m_seriesTK->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }
    for (const auto& point : stPoints) {
        m_seriesST->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }

    // Обновляем масштаб
    updateChart();

    m_chart->setTitle("Исторические данные теста");
}

void ChartWidget::setDataRepository(IDataRepository* repository) {
    m_repository = repository;
    if (m_repository) {
        connect(m_repository, &IDataRepository::dataAdded,
                this, &ChartWidget::updateChart);
    }
}

void ChartWidget::setParameter(const QString& parameter) {
    m_parameter = parameter;
    m_chart->setTitle(QString("Chart for %1").arg(parameter));
    updateChart();
}

void ChartWidget::updateChart() {
    if (!m_repository || !m_recording) {
        return;
    }

    QDateTime to = QDateTime::currentDateTime();
    QDateTime from = to.addSecs(-m_timeRange);

    // Получаем данные для всех трех параметров
    auto pointsAD = m_repository->getDataPoints("AD_RPM", from, to);
    auto pointsTK = m_repository->getDataPoints("TK_RPM", from, to);
    auto pointsST = m_repository->getDataPoints("ST_RPM", from, to);

    // // Обновляем спидометры последними значениями
    // if (!pointsAD.isEmpty()) {
    //     double lastAD = pointsAD.last().value;
    //     // m_speedometerAD->setValue(lastAD);
    // }
    // if (!pointsTK.isEmpty()) {
    //     double lastTK = pointsTK.last().value;
    //     m_speedometerTK->setValue(lastTK);
    // }
    // if (!pointsST.isEmpty()) {
    //     double lastST = pointsST.last().value;
    //     m_speedometerST->setValue(lastST);
    // }

    // Обновляем графики (существующий код)
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();

    double minAD = std::numeric_limits<double>::max();
    double maxAD = std::numeric_limits<double>::min();
    double minTK = std::numeric_limits<double>::max();
    double maxTK = std::numeric_limits<double>::min();
    double minST = std::numeric_limits<double>::max();
    double maxST = std::numeric_limits<double>::min();

    // Добавляем точки для АД
    for (const auto& point : pointsAD) {
        qint64 timestamp = point.timestamp.toMSecsSinceEpoch();
        m_seriesAD->append(timestamp, point.value);
        minAD = qMin(minAD, point.value);
        maxAD = qMax(maxAD, point.value);
    }

    // Добавляем точки для ТК
    for (const auto& point : pointsTK) {
        qint64 timestamp = point.timestamp.toMSecsSinceEpoch();
        m_seriesTK->append(timestamp, point.value);
        minTK = qMin(minTK, point.value);
        maxTK = qMax(maxTK, point.value);
    }

    // Добавляем точки для СТ
    for (const auto& point : pointsST) {
        qint64 timestamp = point.timestamp.toMSecsSinceEpoch();
        m_seriesST->append(timestamp, point.value);
        minST = qMin(minST, point.value);
        maxST = qMax(maxST, point.value);
    }

    // Автомасштабирование осей если включено
    if (m_autoScaleAD->isChecked() && !pointsAD.isEmpty()) {
        double rangeAD = maxAD - minAD;
        if (rangeAD < 1.0) rangeAD = 1.0;
        m_axisY_AD->setRange(minAD - rangeAD * 0.1, maxAD + rangeAD * 0.1);
        m_minAD->setValue(minAD - rangeAD * 0.1);
        m_maxAD->setValue(maxAD + rangeAD * 0.1);
    }

    if (m_autoScaleTK->isChecked() && !pointsTK.isEmpty()) {
        double rangeTK = maxTK - minTK;
        if (rangeTK < 1.0) rangeTK = 1.0;
        m_axisY_TK->setRange(minTK - rangeTK * 0.1, maxTK + rangeTK * 0.1);
        m_minTK->setValue(minTK - rangeTK * 0.1);
        m_maxTK->setValue(maxTK + rangeTK * 0.1);
    }

    if (m_autoScaleST->isChecked() && !pointsST.isEmpty()) {
        double rangeST = maxST - minST;
        if (rangeST < 1.0) rangeST = 1.0;
        m_axisY_ST->setRange(minST - rangeST * 0.1, maxST + rangeST * 0.1);
        m_minST->setValue(minST - rangeST * 0.1);
        m_maxST->setValue(maxST + rangeST * 0.1);
    }

    // Применяем ручной масштаб если нужно
    applyManualScale();

    // Обновляем ось X если есть данные
    if (!pointsAD.isEmpty() || !pointsTK.isEmpty() || !pointsST.isEmpty()) {
        m_axisX->setRange(from, to);
    }
}

```

---


## Файл: ModbusClient/src/gui/widgets/ChartWidget.h

```cpp
#pragma once
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QSplitter>
#include "SpeedometerWidget.h"
#include "data/database/TestSession.h"

QT_CHARTS_USE_NAMESPACE

    class IDataRepository;

class ChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartWidget(QWidget* parent = nullptr);

    void setDataRepository(IDataRepository* repository);
    void setParameter(const QString& parameter);
    void updateChart();
    void setTimeRange(int seconds);
    void setYRange(double min, double max);

    void startTestRecording();
    void stopTestRecording();
    void clearChart();
    void loadHistoricalData(const QVector<DataPointRecord>& points);
    bool isRecording() const { return m_recording; }

private slots:
    void onAutoScaleADChanged(int state);
    void onAutoScaleTKChanged(int state);
    void onAutoScaleSTChanged(int state);
    void onManualScaleADChanged();
    void onManualScaleTKChanged();
    void onManualScaleSTChanged();
    void onTimeRangeChanged();
    void onGridToggled(bool enabled);
    void resetZoom();

private:
    void setupChart();
    void setupMultiAxis();
    void setupControlPanel();
    void setupControlPanelValues();
    void applyManualScale();
    void updateGrid();

    IDataRepository* m_repository;
    QString m_parameter;
    int m_timeRange;
    bool m_recording;

    // QSplitter* m_mainSplitter;

    // Левая панель - спидометры
    // QWidget* m_speedometerPanel;
    // SpeedometerWidget* m_speedometerAD;
    // SpeedometerWidget* m_speedometerTK;
    // SpeedometerWidget* m_speedometerST;

    // Правая панель - графики
    QWidget* m_chartPanel;
    QChart* m_chart;
    QChartView* m_chartView;
    QLineSeries* m_seriesAD;
    QLineSeries* m_seriesTK;
    QLineSeries* m_seriesST;
    QDateTimeAxis* m_axisX;
    QValueAxis* m_axisY_AD;
    QValueAxis* m_axisY_TK;
    QValueAxis* m_axisY_ST;

    // Элементы управления
    QCheckBox* m_autoScaleAD;
    QCheckBox* m_autoScaleTK;
    QCheckBox* m_autoScaleST;
    QDoubleSpinBox* m_minAD;
    QDoubleSpinBox* m_maxAD;
    QDoubleSpinBox* m_minTK;
    QDoubleSpinBox* m_maxTK;
    QDoubleSpinBox* m_minST;
    QDoubleSpinBox* m_maxST;
    QSpinBox* m_timeRangeSpin;
    QCheckBox* m_showGrid;
    QPushButton* m_resetZoomButton;
};

```

---


## Файл: ModbusClient/src/gui/monitoring/MonitoringViewController.h

```cpp
#pragma once
#include <QObject>
#include "monitoring/DataMonitor.h"
#include "control/ControlStateMachine.h"
#include "control/ControlUIController.h"
#include "control/ModeController.h"
#include "../widgets/MonitorWidget.h"
#include "../widgets/ChartWidget.h"
#include "../factories/WidgetFactory.h"

class MonitoringViewController : public QObject {
    Q_OBJECT
public:
    explicit MonitoringViewController(DataMonitor* dataMonitor,
                                    ControlStateMachine* controlStateMachine,
                                    ModeController* modeController,
                                    IDataRepository* dataRepository,
                                    QObject* parent = nullptr);
    ~MonitoringViewController() override = default;

    QWidget* getWidget();

    void startMonitoring();
    void stopMonitoring();
    void setRecording(bool recording);

signals:
    void exportRequested();

private slots:
    void onLogMessage(const QString& message);
    void onTestStarted(const QString& mode);
    void onTestStopped();
    void onExportClicked();

private:
    void setupUI();
    void setupConnections();

    DataMonitor* m_dataMonitor;
    ControlStateMachine* m_controlStateMachine;
    ModeController* m_modeController;
    IDataRepository* m_dataRepository;
    ControlUIController* m_controlUIController;

    QWidget* m_mainWidget;
    MonitorWidget* m_monitorWidget;
    ChartWidget* m_chartWidget;
    WidgetFactory* m_widgetFactory;
    QPushButton* m_exportButton;
};

```

---


## Файл: ModbusClient/src/gui/monitoring/MonitoringViewController.cpp

```cpp
#include "MonitoringViewController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QTextEdit>
#include <QDebug>

MonitoringViewController::MonitoringViewController(DataMonitor* dataMonitor,
                                                   ControlStateMachine* controlStateMachine,
                                                   ModeController* modeController,
                                                   IDataRepository* dataRepository,
                                                   QObject* parent)
    : QObject(parent)
    , m_dataMonitor(dataMonitor)
    , m_controlStateMachine(controlStateMachine)
    , m_modeController(modeController)
    , m_dataRepository(dataRepository)
    , m_controlUIController(nullptr)
    , m_mainWidget(nullptr)
    , m_monitorWidget(nullptr)
    , m_chartWidget(nullptr)
    , m_widgetFactory(nullptr)
    , m_exportButton(nullptr)
{
    setupUI();
    setupConnections();
}

QWidget* MonitoringViewController::getWidget() {
    return m_mainWidget;
}

void MonitoringViewController::setupUI() {
    m_mainWidget = new QWidget();
    m_widgetFactory = new WidgetFactory(m_mainWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(m_mainWidget);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    // Левая панель - monitoring
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_monitorWidget = m_widgetFactory->createMonitorWidget();
    leftLayout->addWidget(m_monitorWidget);
    leftLayout->addStretch();

    // Правая панель - chart
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    m_chartWidget = m_widgetFactory->createChartWidget(m_dataRepository);
    m_exportButton = new QPushButton("Экспорт графика");
    m_exportButton->setMinimumHeight(35);

    rightLayout->addWidget(m_chartWidget);
    rightLayout->addWidget(m_exportButton);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({400, 800});

    mainLayout->addWidget(splitter);
}

void MonitoringViewController::setupConnections() {
    if (!m_monitorWidget || !m_controlStateMachine) {
        qCritical() << "MonitoringViewController: Critical components are null!";
        return;
    }

    // Создаем UI контроллер для управления кнопками
    m_controlUIController = new ControlUIController(m_controlStateMachine, this);
    m_controlUIController->setControlButtons(
        m_monitorWidget->actionButton1(),
        m_monitorWidget->actionButton2()
    );

    // Подключаем сигналы мониторинга
    if (m_dataMonitor && m_monitorWidget) {
        m_monitorWidget->setDiscreteMonitor(m_dataMonitor->discreteMonitor());
        m_monitorWidget->setAnalogMonitor(m_dataMonitor->analogMonitor());
    }

    // Подключаем сигналы режимов
    if (m_modeController) {
        connect(m_modeController, &ModeController::logMessage,
                this, &MonitoringViewController::onLogMessage);
        connect(m_modeController, &ModeController::testStarted,
                this, &MonitoringViewController::onTestStarted);
        connect(m_modeController, &ModeController::testStopped,
                this, &MonitoringViewController::onTestStopped);
    }

    // Подключаем экспорт
    connect(m_exportButton, &QPushButton::clicked,
            this, &MonitoringViewController::onExportClicked);

    // Подключаем state machine к графику
    if (m_controlStateMachine && m_chartWidget) {
        connect(m_controlStateMachine, &ControlStateMachine::startChartRecording,
                m_chartWidget, &ChartWidget::startTestRecording);
        connect(m_controlStateMachine, &ControlStateMachine::stopChartRecording,
                m_chartWidget, &ChartWidget::stopTestRecording);
    }

    qDebug() << "MonitoringViewController: All connections established";
}

void MonitoringViewController::startMonitoring() {
    if (m_dataMonitor) {
        m_dataMonitor->startMonitoring();
        qDebug() << "MonitoringViewController: Monitoring started";
    }
}

void MonitoringViewController::stopMonitoring() {
    if (m_dataMonitor) {
        m_dataMonitor->stopMonitoring();
        qDebug() << "MonitoringViewController: Monitoring stopped";
    }
}

void MonitoringViewController::setRecording(bool recording) {
    if (recording) {
        if (m_chartWidget) {
            m_chartWidget->startTestRecording();
        }
    } else {
        if (m_chartWidget) {
            m_chartWidget->stopTestRecording();
        }
    }
}

void MonitoringViewController::onLogMessage(const QString& message) {
    qDebug() << "Monitoring:" << message;

    // Если есть лог в монитор виджете, добавляем туда
    if (m_monitorWidget && m_monitorWidget->logTextEdit()) {
        m_monitorWidget->logTextEdit()->append(message);
    }
}

void MonitoringViewController::onTestStarted(const QString& mode) {
//    setRecording(true);
    onLogMessage(QString("Тест запущен: %1").arg(mode));
}

void MonitoringViewController::onTestStopped() {
    setRecording(false);
    onLogMessage("Тест остановлен");
}

void MonitoringViewController::onExportClicked() {
    if (m_chartWidget) {
        onLogMessage("Экспорт графика запрошен");
        emit exportRequested();
    }
}

```

---


## Файл: ModbusClient/src/gui/mainwindow/MainWindow.cpp

```cpp
#include "MainWindow.h"
#include "../connection/ConnectionViewController.h"
#include "../mode_selection/ModeSelectionViewController.h"
#include "../monitoring/MonitoringViewController.h"
#include "../database/DatabaseViewController.h"
#include "../widgets/ChartWidget.h"
#include "data/interfaces/IDataRepository.h"
#include <QSplitter>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_mainSplitter(nullptr)
    , m_tabWidget(nullptr)
    , m_connectionVC(nullptr)
    , m_modeSelectionVC(nullptr)
    , m_monitoringVC(nullptr)
    , m_databaseVC(nullptr)
    , m_dataRepository(nullptr)
{
    qDebug() << "MainWindow: Created";
}

void MainWindow::setupUI(ConnectionViewController* connectionVC,
                         ModeSelectionViewController* modeSelectionVC,
                         MonitoringViewController* monitoringVC,
                         DatabaseViewController* databaseVC) {
    m_connectionVC = connectionVC;
    m_modeSelectionVC = modeSelectionVC;
    m_monitoringVC = monitoringVC;
    m_databaseVC = databaseVC;

    qDebug() << "MainWindow: Setting up UI";

    // Главный splitter - левая панель (вкладки) и правая панель (график)
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);

    // Левая панель - вкладки
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_tabWidget = new QTabWidget();

    // Добавляем вкладки
    if (m_connectionVC && m_modeSelectionVC) {
        QWidget* controlTab = new QWidget();
        QVBoxLayout* controlLayout = new QVBoxLayout(controlTab);

        // Вкладка управления: подключение + выбор режима
        controlLayout->addWidget(m_connectionVC->getWidget());
        controlLayout->addWidget(m_modeSelectionVC->getWidget());
        controlLayout->addStretch();

        m_tabWidget->addTab(controlTab, "Управление");
        qDebug() << "MainWindow: Added Control tab";
    }

    if (m_monitoringVC) {
        m_tabWidget->addTab(m_monitoringVC->getWidget(), "Мониторинг");
        qDebug() << "MainWindow: Added Monitoring tab";
    }

    if (m_databaseVC) {
        m_tabWidget->addTab(m_databaseVC->getWidget(), "История");
        qDebug() << "MainWindow: Added Database tab";
    }

    leftLayout->addWidget(m_tabWidget);

    // Добавляем панели в splitter
    m_mainSplitter->addWidget(leftPanel);

    setCentralWidget(m_mainSplitter);
    setWindowTitle("Delta AS332T Modbus Client");
    showMaximized();

    qDebug() << "MainWindow: UI setup completed";
}

void MainWindow::setupConnections() {
    // Здесь можно добавить связи между компонентами, если нужно
    qDebug() << "MainWindow: Setting up connections";
}

void MainWindow::closeEvent(QCloseEvent* event) {
    qDebug() << "MainWindow: Close event triggered";

    // Показываем диалог подтверждения
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Выход",
                                  "Вы уверены, что хотите выйти?\nВсе несохранённые данные будут сохранены.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        event->ignore();
        qDebug() << "MainWindow: Close cancelled by user";
        return;
    }

    // Сохраняем данные перед закрытием
    if (m_dataRepository) {
        qDebug() << "MainWindow: Finalizing data session";
        m_dataRepository->finalizeSession();
    }

    // Останавливаем мониторинг
    if (m_monitoringVC) {
        qDebug() << "MainWindow: Stopping monitoring";
        m_monitoringVC->stopMonitoring();
    }

    qDebug() << "MainWindow: Application closing";
    event->accept();
    QMainWindow::closeEvent(event);
}

```

---


## Файл: ModbusClient/src/gui/mainwindow/MainWindow.h

```cpp
#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QCloseEvent>

class QTabWidget;
class ConnectionViewController;
class ModeSelectionViewController;
class MonitoringViewController;
class DatabaseViewController;
class IDataRepository;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void setupUI(ConnectionViewController* connectionVC,
                 ModeSelectionViewController* modeSelectionVC,
                 MonitoringViewController* monitoringVC,
                 DatabaseViewController* databaseVC);

    void setDataRepository(IDataRepository* repository) { m_dataRepository = repository; }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupConnections();

    QSplitter* m_mainSplitter;
    QTabWidget* m_tabWidget;

    ConnectionViewController* m_connectionVC;
    ModeSelectionViewController* m_modeSelectionVC;
    MonitoringViewController* m_monitoringVC;
    DatabaseViewController* m_databaseVC;
    IDataRepository* m_dataRepository;
};

```

---


## Файл: ModbusClient/src/gui/CMakeLists.txt

```cpp
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


```

---


## Файл: ModbusClient/src/gui/factories/WidgetFactory.h

```cpp
#pragma once
#include <QObject>

class IModbusClient;
class IDataRepository;
class ConnectionWidget;
class MonitorWidget;
class ChartWidget;
class QComboBox;
class QPushButton;
class QLineEdit;
class QTextEdit;

class WidgetFactory : public QObject {
    Q_OBJECT
public:
    explicit WidgetFactory(QObject* parent = nullptr);

    ConnectionWidget* createConnectionWidget();
    MonitorWidget* createMonitorWidget();
    ChartWidget* createChartWidget(IDataRepository* repository);

    QWidget* createStateControlWidget();

    QWidget* createModeControlWidget(); // Теперь возвращает 6 кнопок
    QWidget* createParameterControlWidget();
    QTextEdit* createLogWidget();

    // Новые методы для создания элементов управления
    QWidget* createControlButtonsWidget();
    QTextEdit* createMonitorLogWidget();

    // Getters for controls - теперь для кнопок режимов
    QPushButton* modeButton1() const { return m_modeButton1; }
    QPushButton* modeButton2() const { return m_modeButton2; }
    QPushButton* modeButton3() const { return m_modeButton3; }
    QPushButton* modeButton4() const { return m_modeButton4; }
    QPushButton* modeButton5() const { return m_modeButton5; }
    QPushButton* stopButton() const { return m_stopButton; }

    // Добавляем геттеры для новых кнопок
    QPushButton* actionButton1() const { return m_actionButton1; }
    QPushButton* actionButton2() const { return m_actionButton2; }


    QComboBox* parameterComboBox() const { return m_paramCombo; }
    QLineEdit* addressEdit() const { return m_addressEdit; }
    QPushButton* addParameterButton() const { return m_addParamButton; }

    // Getters for control buttons
    QPushButton* readyCheckButton() const { return m_readyCheckButton; }
    QPushButton* controlStartButton() const { return m_controlStartButton; }
    QPushButton* restartButton() const { return m_restartButton; }
    QPushButton* exitButton() const { return m_exitButton; }

private:
    // Кнопки режимов вместо ComboBox
    QPushButton* m_modeButton1;
    QPushButton* m_modeButton2;
    QPushButton* m_modeButton3;
    QPushButton* m_modeButton4;
    QPushButton* m_modeButton5;
    QPushButton* m_stopButton;

    QComboBox* m_paramCombo;
    QLineEdit* m_addressEdit;
    QPushButton* m_addParamButton;

    // Кнопки управления для мониторинга
    QPushButton* m_readyCheckButton;
    QPushButton* m_controlStartButton;
    QPushButton* m_restartButton;
    QPushButton* m_exitButton;

    QPushButton* m_actionButton1;
    QPushButton* m_actionButton2;
};

```

---


## Файл: ModbusClient/src/gui/factories/WidgetFactory.cpp

```cpp
#include "WidgetFactory.h"
#include "../widgets/ConnectionWidget.h"
#include "../widgets/MonitorWidget.h"
#include "../widgets/ChartWidget.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QFormLayout>
#include <QGridLayout>

WidgetFactory::WidgetFactory(QObject* parent)
    : QObject(parent)
    , m_modeButton1(nullptr)
    , m_modeButton2(nullptr)
    , m_modeButton3(nullptr)
    , m_modeButton4(nullptr)
    , m_modeButton5(nullptr)
    , m_stopButton(nullptr)
    , m_paramCombo(nullptr)
    , m_addressEdit(nullptr)
    , m_addParamButton(nullptr)
    , m_readyCheckButton(nullptr)
    , m_controlStartButton(nullptr)
    , m_restartButton(nullptr)
    , m_exitButton(nullptr)
{}

ConnectionWidget* WidgetFactory::createConnectionWidget() {
    return new ConnectionWidget();
}

MonitorWidget* WidgetFactory::createMonitorWidget() {
    return new MonitorWidget();
}

ChartWidget* WidgetFactory::createChartWidget(IDataRepository* repository) {
    ChartWidget* widget = new ChartWidget();
    widget->setDataRepository(repository);
    return widget;
}

QWidget* WidgetFactory::createModeControlWidget() {
    QGroupBox* group = new QGroupBox("Управление режимами тестирования");
    QGridLayout* layout = new QGridLayout(group);

    // Создаем 5 кнопок режимов и 1 кнопку остановки
    m_modeButton1 = new QPushButton("Расконсервация/\nКонсервация");
    m_modeButton2 = new QPushButton("Холодная прокрутка\nтурбостартера");
    m_modeButton3 = new QPushButton("Регулировка мощности,\nзамер параметров");
    m_modeButton4 = new QPushButton("Холодная прокрутка\nосновного двигателя");
    m_modeButton5 = new QPushButton("Имитация запуска\nосновного двигателя");
    m_stopButton = new QPushButton("СТОП ТЕСТА");

    // Настраиваем размеры кнопок
    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
    for (QPushButton* button : modeButtons) {
        button->setMinimumSize(180, 80);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    m_stopButton->setMinimumSize(180, 80);
    m_stopButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Настраиваем стили для неактивного состояния (по умолчанию)
    QString disabledStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #7f8c8d;"
        "border-radius: 8px;"
        "background-color: #bdc3c7;"
        "color: #7f8c8d;"
        "}";

    QString activeModeStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 8px;"
        "background-color: #3498db;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}";

    QString activeStopStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #c0392b;"
        "border-radius: 8px;"
        "background-color: #e74c3c;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "background-color: #a93226;"
        "}";

    QString activeGreenStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #27ae60;"
        "border-radius: 8px;"
        "background-color: #2ecc71;"
        "color: white;"
        "}";

    // Применяем неактивный стиль по умолчанию
    for (QPushButton* button : modeButtons) {
        button->setStyleSheet(disabledStyle);
        button->setEnabled(false);
    }
    m_stopButton->setStyleSheet(disabledStyle);
    m_stopButton->setEnabled(false);

    // Сохраняем стили как свойства для последующего использования
    m_modeButton1->setProperty("activeStyle", activeModeStyle);
    m_modeButton1->setProperty("disabledStyle", disabledStyle);
    m_modeButton1->setProperty("activeGreenStyle", activeGreenStyle);

    // Располагаем кнопки в сетке 2x3
    layout->addWidget(m_modeButton1, 0, 0);
    layout->addWidget(m_modeButton2, 0, 1);
    layout->addWidget(m_modeButton3, 0, 2);
    layout->addWidget(m_modeButton4, 1, 0);
    layout->addWidget(m_modeButton5, 1, 1);
    layout->addWidget(m_stopButton, 1, 2);

    // Устанавливаем растягивание столбцов
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 1);

    return group;
}

QWidget* WidgetFactory::createStateControlWidget() {
    QGroupBox* group = new QGroupBox("Управление запуском");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем только две кнопки для всех состояний
    m_actionButton1 = new QPushButton("Проверка готовности");
    m_actionButton2 = new QPushButton("Выход");

    // Настраиваем размеры кнопок
    m_actionButton1->setMinimumHeight(40);
    m_actionButton2->setMinimumHeight(40);

    // Базовый стиль
    QString buttonStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 12px;"
        "padding: 8px 12px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 5px;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "background-color: #bdc3c7;"
        "border-color: #95a5a6;"
        "color: #7f8c8d;"
        "}";

    m_actionButton1->setStyleSheet(buttonStyle + "background-color: #3498db;");
    m_actionButton2->setStyleSheet(buttonStyle + "background-color: #e74c3c;");

    layout->addWidget(m_actionButton1);
    layout->addWidget(m_actionButton2);

    return group;
}


QWidget* WidgetFactory::createParameterControlWidget() {
    return nullptr;
}

QTextEdit* WidgetFactory::createLogWidget() {
    QTextEdit* logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(200);
    return logEdit;
}

QWidget* WidgetFactory::createControlButtonsWidget() {
    QGroupBox* group = new QGroupBox("Управление запуском");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем кнопки управления (теперь 4 вместо 5)
    m_readyCheckButton = new QPushButton("Проверка готовности");
    m_controlStartButton = new QPushButton("ПУСК"); // Будет меняться на СТОП
    m_restartButton = new QPushButton("Повторение запуска"); // Будет меняться на Прерывание
    m_exitButton = new QPushButton("ВЫХОД");

    // Настраиваем размеры кнопок
    m_readyCheckButton->setMinimumHeight(40);
    m_controlStartButton->setMinimumHeight(40);
    m_restartButton->setMinimumHeight(40);
    m_exitButton->setMinimumHeight(40);

    // Базовый стиль
    QString buttonStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 12px;"
        "padding: 8px 12px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 5px;"
        "background-color: #3498db;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "background-color: #bdc3c7;"
        "border-color: #95a5a6;"
        "color: #7f8c8d;"
        "}";

    m_readyCheckButton->setStyleSheet(buttonStyle);
    m_controlStartButton->setStyleSheet(buttonStyle);
    m_restartButton->setStyleSheet(buttonStyle);
    m_exitButton->setStyleSheet(buttonStyle);

    // Начальное состояние кнопок
    m_readyCheckButton->setEnabled(true);
    m_controlStartButton->setEnabled(true);
    m_restartButton->setEnabled(true);
    m_exitButton->setEnabled(true);

    // Добавляем кнопки в layout
    layout->addWidget(m_readyCheckButton);
    layout->addWidget(m_controlStartButton);
    layout->addWidget(m_restartButton);
    layout->addWidget(m_exitButton);

    return group;
}

QTextEdit* WidgetFactory::createMonitorLogWidget() {
    QTextEdit* logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(150);
    logEdit->setPlaceholderText("Журнал событий мониторинга...");
    return logEdit;
}

```

---


## Файл: ModbusClient/src/gui/database/DatabaseViewController.cpp

```cpp
#include "DatabaseViewController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QDateEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>

DatabaseViewController::DatabaseViewController(DatabaseAsyncManager* dbManager,
                                               DatabaseExportService* exportService,
                                               QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
    , m_exportService(exportService)
    , m_mainWidget(nullptr)
    , m_sessionsTable(nullptr)
    , m_fromDateEdit(nullptr)
    , m_toDateEdit(nullptr)
    , m_testTypeCombo(nullptr)
    , m_loadSessionsButton(nullptr)
    , m_loadDataButton(nullptr)
    , m_exportCsvButton(nullptr)
    , m_exportImageButton(nullptr)
    , m_statusLabel(nullptr)
    , m_currentSessionId(-1)
{
    setupUI();
    setupConnections();
}

QWidget* DatabaseViewController::getWidget() {
    return m_mainWidget;
}

void DatabaseViewController::setupUI() {
    m_mainWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(m_mainWidget);

    // Заголовок
    QLabel* titleLabel = new QLabel("История тестов");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Панель поиска
    QGroupBox* searchGroup = new QGroupBox("Поиск тестовых сессий");
    QFormLayout* searchLayout = new QFormLayout(searchGroup);

    m_fromDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    m_fromDateEdit->setCalendarPopup(true);
    m_toDateEdit = new QDateEdit(QDate::currentDate());
    m_toDateEdit->setCalendarPopup(true);

    m_testTypeCombo = new QComboBox();
    m_testTypeCombo->addItem("Все типы", "");
    m_testTypeCombo->addItem("Расконсервация/Консервация", "Расконсервация/Консервация");
    m_testTypeCombo->addItem("Холодная прокрутка турбостартера", "Холодная прокрутка турбостартера");
    m_testTypeCombo->addItem("Регулировка мощности, замер параметров", "Регулировка мощности, замер параметров");
    m_testTypeCombo->addItem("Холодная прокрутка основного двигателя", "Холодная прокрутка основного двигателя");
    m_testTypeCombo->addItem("Имитация запуска основного двигателя", "Имитация запуска основного двигателя");

    m_loadSessionsButton = new QPushButton("Загрузить сессии");

    searchLayout->addRow("Период с:", m_fromDateEdit);
    searchLayout->addRow("по:", m_toDateEdit);
    searchLayout->addRow("Тип теста:", m_testTypeCombo);
    searchLayout->addRow(m_loadSessionsButton);

    mainLayout->addWidget(searchGroup);

    // Таблица сессий
    QGroupBox* sessionsGroup = new QGroupBox("Тестовые сессии");
    QVBoxLayout* sessionsLayout = new QVBoxLayout(sessionsGroup);

    m_sessionsTable = new QTableWidget();
    m_sessionsTable->setColumnCount(5);
    m_sessionsTable->setHorizontalHeaderLabels({
        "ID", "Тип теста", "Начало", "Окончание", "Длительность"
    });
    m_sessionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sessionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sessionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Настройка ширины колонок
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_sessionsTable->setColumnWidth(0, 50);
    m_sessionsTable->setColumnWidth(2, 150);
    m_sessionsTable->setColumnWidth(3, 150);
    m_sessionsTable->setColumnWidth(4, 100);

    sessionsLayout->addWidget(m_sessionsTable);

    // Кнопки управления выбранной сессией
    QHBoxLayout* sessionButtonsLayout = new QHBoxLayout();
    m_loadDataButton = new QPushButton("Загрузить данные");
    m_exportCsvButton = new QPushButton("Экспорт в CSV");
    m_exportImageButton = new QPushButton("Экспорт в изображение");

    m_loadDataButton->setEnabled(false);
    m_exportCsvButton->setEnabled(false);
    m_exportImageButton->setEnabled(false);

    sessionButtonsLayout->addWidget(m_loadDataButton);
    sessionButtonsLayout->addWidget(m_exportCsvButton);
    sessionButtonsLayout->addWidget(m_exportImageButton);
    sessionButtonsLayout->addStretch();

    sessionsLayout->addLayout(sessionButtonsLayout);
    mainLayout->addWidget(sessionsGroup);

    // Статусная строка
    m_statusLabel = new QLabel("Готов к работе");
    m_statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();
}

void DatabaseViewController::setupConnections() {
    connect(m_loadSessionsButton, &QPushButton::clicked,
            this, &DatabaseViewController::onLoadSessionsClicked);
    connect(m_loadDataButton, &QPushButton::clicked,
            this, &DatabaseViewController::onLoadDataClicked);
    connect(m_exportCsvButton, &QPushButton::clicked,
            this, &DatabaseViewController::onExportCsvClicked);
    connect(m_exportImageButton, &QPushButton::clicked,
            this, &DatabaseViewController::onExportImageClicked);
    connect(m_sessionsTable, &QTableWidget::itemSelectionChanged,
            this, &DatabaseViewController::onSessionSelectionChanged);

    if (m_exportService) {
        connect(m_exportService, &DatabaseExportService::exportCompleted,
                this, &DatabaseViewController::onExportCompleted);
        connect(m_exportService, &DatabaseExportService::exportFailed,
                this, &DatabaseViewController::onExportFailed);
    }
}

void DatabaseViewController::onLoadSessionsClicked() {
    if (!m_dbManager) {
        showMessage("Ошибка: менеджер БД не инициализирован");
        return;
    }

    QDateTime from = QDateTime(m_fromDateEdit->date(), QTime(0, 0, 0));
    QDateTime to = QDateTime(m_toDateEdit->date(), QTime(23, 59, 59));
    QString testType = m_testTypeCombo->currentData().toString();

    showMessage(QString("Загрузка сессий с %1 по %2").arg(
        from.toString("dd.MM.yyyy"), to.toString("dd.MM.yyyy")));

    emit loadSessionsRequested(from, to, testType);
}

void DatabaseViewController::onLoadDataClicked() {
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(nullptr, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    showMessage(QString("Загрузка данных для сессии ID: %1").arg(m_currentSessionId));
    emit loadSessionDataRequested(m_currentSessionId);
}

void DatabaseViewController::onExportCsvClicked() {
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(nullptr, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        nullptr, "Экспорт в CSV",
        QString("session_%1.csv").arg(m_currentSessionId),
        "CSV Files (*.csv)");

    if (!filename.isEmpty()) {
        showMessage(QString("Экспорт сессии %1 в CSV: %2").arg(m_currentSessionId).arg(filename));
        emit exportToCsvRequested(m_currentSessionId, filename);
    }
}

void DatabaseViewController::onExportImageClicked() {
    if (m_currentSessionId <= 0) {
        QMessageBox::warning(nullptr, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        nullptr, "Экспорт в изображение",
        QString("session_%1.png").arg(m_currentSessionId),
        "PNG Files (*.png);;JPEG Files (*.jpg)");

    if (!filename.isEmpty()) {
        showMessage(QString("Экспорт сессии %1 в изображение: %2").arg(m_currentSessionId).arg(filename));
        emit exportToImageRequested(m_currentSessionId, filename);
    }
}

void DatabaseViewController::onSessionSelectionChanged() {
    QList<QTableWidgetItem*> selectedItems = m_sessionsTable->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();

    m_loadDataButton->setEnabled(hasSelection);
    m_exportCsvButton->setEnabled(hasSelection);
    m_exportImageButton->setEnabled(hasSelection);

    if (hasSelection) {
        int row = selectedItems.first()->row();
        QTableWidgetItem* idItem = m_sessionsTable->item(row, 0);
        m_currentSessionId = idItem->data(Qt::UserRole).toInt();
        showMessage(QString("Выбрана сессия ID: %1").arg(m_currentSessionId));
    } else {
        m_currentSessionId = -1;
    }
}

void DatabaseViewController::onExportCompleted(const QString& filename) {
    showMessage(QString("Экспорт успешно завершен: %1").arg(filename));
    QMessageBox::information(nullptr, "Экспорт",
                           QString("Данные успешно экспортированы в:\n%1").arg(filename));
}

void DatabaseViewController::onExportFailed(const QString& error) {
    showMessage(QString("Ошибка экспорта: %1").arg(error));
    QMessageBox::critical(nullptr, "Ошибка экспорта", error);
}

void DatabaseViewController::showSessions(const QVector<TestSession>& sessions) {
    updateSessionsTable(sessions);
    showMessage(QString("Загружено %1 тестовых сессий").arg(sessions.size()));
}

void DatabaseViewController::showSessionData(const QVector<DataPointRecord>& points) {
    if (!points.isEmpty()) {
        showMessage(QString("Загружено %1 точек данных").arg(points.size()));

        // Анализ данных
        QMap<QString, int> pointsByParameter;
        QDateTime minTime, maxTime;

        for (const auto& point : points) {
            pointsByParameter[point.parameter]++;

            if (!minTime.isValid() || point.timestamp < minTime) {
                minTime = point.timestamp;
            }
            if (!maxTime.isValid() || point.timestamp > maxTime) {
                maxTime = point.timestamp;
            }
        }

        QStringList stats;
        for (auto it = pointsByParameter.begin(); it != pointsByParameter.end(); ++it) {
            stats.append(QString("%1: %2 точек").arg(it.key()).arg(it.value()));
        }

        showMessage(QString("Период данных: %1 - %2 | %3")
                   .arg(minTime.toString("dd.MM.yyyy HH:mm:ss"))
                   .arg(maxTime.toString("dd.MM.yyyy HH:mm:ss"))
                   .arg(stats.join(", ")));
    } else {
        QMessageBox::information(nullptr, "Информация", "Для выбранной сессии нет данных");
    }
}

void DatabaseViewController::showExportProgress(int progress) {
    showMessage(QString("Прогресс экспорта: %1%").arg(progress));
}

void DatabaseViewController::showMessage(const QString& message) {
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
}

int DatabaseViewController::getSelectedSessionId() const {
    return m_currentSessionId;
}

QDateTime DatabaseViewController::getSearchFromDate() const {
    return m_fromDateEdit ? QDateTime(m_fromDateEdit->date(), QTime(0, 0, 0)) : QDateTime();
}

QDateTime DatabaseViewController::getSearchToDate() const {
    return m_toDateEdit ? QDateTime(m_toDateEdit->date(), QTime(23, 59, 59)) : QDateTime();
}

QString DatabaseViewController::getSearchTestType() const {
    return m_testTypeCombo ? m_testTypeCombo->currentData().toString() : "";
}

void DatabaseViewController::updateSessionsTable(const QVector<TestSession>& sessions) {
    if (!m_sessionsTable) return;

    m_sessionsTable->setRowCount(sessions.size());

    for (int i = 0; i < sessions.size(); ++i) {
        const TestSession& session = sessions[i];

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(session.id));
        idItem->setData(Qt::UserRole, session.id);
        m_sessionsTable->setItem(i, 0, idItem);

        // Тип теста
        m_sessionsTable->setItem(i, 1, new QTableWidgetItem(session.testType));

        // Время начала
        m_sessionsTable->setItem(i, 2, new QTableWidgetItem(
            session.startTime.toString("dd.MM.yyyy HH:mm:ss")));

        // Время окончания
        QString endTimeStr = session.endTime.isValid() ?
            session.endTime.toString("dd.MM.yyyy HH:mm:ss") : "Не завершена";
        m_sessionsTable->setItem(i, 3, new QTableWidgetItem(endTimeStr));

        // Длительность
        QString durationStr = formatDuration(session.startTime, session.endTime);
        m_sessionsTable->setItem(i, 4, new QTableWidgetItem(durationStr));
    }
}

QString DatabaseViewController::formatDuration(const QDateTime& start, const QDateTime& end) const {
    if (!end.isValid() || !start.isValid()) {
        return "N/A";
    }

    qint64 duration = start.secsTo(end);
    if (duration <= 0) {
        return "N/A";
    }

    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;
    int seconds = duration % 60;

    return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}

```

---


## Файл: ModbusClient/src/gui/database/DatabaseViewController.h

```cpp
#pragma once
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/DatabaseExportService.h"
#include <QObject>
#include <QWidget>

class QTableWidget;
class QDateEdit;
class QComboBox;
class QPushButton;
class QLabel;

class DatabaseViewController : public QObject {
    Q_OBJECT
public:
    explicit DatabaseViewController(DatabaseAsyncManager* dbManager,
                                  DatabaseExportService* exportService,
                                  QObject* parent = nullptr);
    ~DatabaseViewController() override = default;

    // IDatabaseView interface
    QWidget* getWidget();

    void showSessions(const QVector<TestSession>& sessions);
    void showSessionData(const QVector<DataPointRecord>& points);
    void showExportProgress(int progress);
    void showMessage(const QString& message);

    int getSelectedSessionId() const;
    QDateTime getSearchFromDate() const;
    QDateTime getSearchToDate() const;
    QString getSearchTestType() const;

signals:
    // Сигналы IDatabaseView
    void loadSessionsRequested(const QDateTime& from, const QDateTime& to, const QString& testType);
    void loadSessionDataRequested(int sessionId);
    void exportToCsvRequested(int sessionId, const QString& filename);
    void exportToImageRequested(int sessionId, const QString& filename);

private slots:
    void onLoadSessionsClicked();
    void onLoadDataClicked();
    void onExportCsvClicked();
    void onExportImageClicked();
    void onSessionSelectionChanged();
    void onExportCompleted(const QString& filename);
    void onExportFailed(const QString& error);

private:
    void setupUI();
    void setupConnections();
    void updateSessionsTable(const QVector<TestSession>& sessions);
    QString formatDuration(const QDateTime& start, const QDateTime& end) const;

    DatabaseAsyncManager* m_dbManager;
    DatabaseExportService* m_exportService;
    QWidget* m_mainWidget;
    QTableWidget* m_sessionsTable;
    QDateEdit* m_fromDateEdit;
    QDateEdit* m_toDateEdit;
    QComboBox* m_testTypeCombo;
    QPushButton* m_loadSessionsButton;
    QPushButton* m_loadDataButton;
    QPushButton* m_exportCsvButton;
    QPushButton* m_exportImageButton;
    QLabel* m_statusLabel;

    int m_currentSessionId;
};

```

---


## Файл: ModbusClient/src/gui/connection/ConnectionViewController.h

```cpp
#pragma once
#include "../widgets/ConnectionWidget.h"
#include <QObject>

class ConnectionManager;

class ConnectionViewController : public QObject {
    Q_OBJECT
public:
    explicit ConnectionViewController(ConnectionManager* connectionManager, QObject* parent = nullptr);
    ~ConnectionViewController() override = default;

    // IConnectionView interface
    QWidget* getWidget();

    void setConnectionStatus(const QString& status);
    void setConnected(bool connected);
//    void setServerAddress(const QString& address);
//    void setServerPort(quint16 port);
//    void setLocalPort(quint16 port);

    QString serverAddress() const;
    quint16 serverPort() const;
    quint16 localPort() const;

signals:
    void connectRequested(const QString& address, quint16 serverPort, quint16 clientPort);
    void disconnectRequested();
    void localPortChanged(quint16 port);

public slots:
    void onConnectRequested(const QString& address, quint16 serverPort, quint16 clientPort);
    void onDisconnectRequested();
    void onConnectionStatusChanged(const QString& status);
    void onErrorOccurred(const QString& error);

private:
    void setupConnections();

    ConnectionManager* m_connectionManager;
    ConnectionWidget* m_connectionWidget;
};

```

---


## Файл: ModbusClient/src/gui/connection/ConnectionViewController.cpp

```cpp
#include "ConnectionViewController.h"
#include "core/connection/ConnectionManager.h"
#include <QMessageBox>

ConnectionViewController::ConnectionViewController(ConnectionManager* connectionManager,
                                                   QObject* parent)
    : QObject(parent)
    , m_connectionManager(connectionManager)
    , m_connectionWidget(new ConnectionWidget())
{
    setupConnections();
}

QWidget* ConnectionViewController::getWidget() {
    return m_connectionWidget;
}

void ConnectionViewController::setupConnections() {
    if (!m_connectionWidget || !m_connectionManager) {
        return;
    }

    // View -> Controller
    connect(m_connectionWidget, &ConnectionWidget::connectRequested,
            this, &ConnectionViewController::onConnectRequested);
    connect(m_connectionWidget, &ConnectionWidget::disconnectRequested,
            this, &ConnectionViewController::onDisconnectRequested);

    // Controller -> Model
    connect(this, &ConnectionViewController::connectRequested,
            m_connectionManager, &ConnectionManager::connectToDevice);
    connect(this, &ConnectionViewController::disconnectRequested,
            m_connectionManager, &ConnectionManager::disconnectFromDevice);

    // Model -> View
    connect(m_connectionManager, &ConnectionManager::connectionStatusChanged,
            this, &ConnectionViewController::onConnectionStatusChanged);
    connect(m_connectionManager, &ConnectionManager::errorOccurred,
            this, &ConnectionViewController::onErrorOccurred);
}

void ConnectionViewController::onConnectRequested(const QString& address,
                                                 quint16 serverPort,
                                                 quint16 clientPort) {
    emit connectRequested(address, serverPort, clientPort);
}

void ConnectionViewController::onDisconnectRequested() {
    emit disconnectRequested();
}

void ConnectionViewController::onConnectionStatusChanged(const QString& status) {
    if (m_connectionWidget) {
        m_connectionWidget->setStatus(status);
        bool connected = (status == "ПОДКЛЮЧЕН");
        m_connectionWidget->setConnected(connected);
    }
}

void ConnectionViewController::onErrorOccurred(const QString& error) {
    QMessageBox::warning(nullptr, "Ошибка подключения", error);
}

void ConnectionViewController::setConnectionStatus(const QString& status) {
    if (m_connectionWidget) {
        m_connectionWidget->setStatus(status);
    }
}

void ConnectionViewController::setConnected(bool connected) {
    if (m_connectionWidget) {
        m_connectionWidget->setConnected(connected);
    }
}

//void ConnectionViewController::setServerAddress(const QString& address) {
//    // Реализация через ConnectionWidget
//}

//void ConnectionViewController::setServerPort(quint16 port) {
//    // Реализация через ConnectionWidget
//}

//void ConnectionViewController::setLocalPort(quint16 port) {
//    // Реализация через ConnectionWidget
//}

QString ConnectionViewController::serverAddress() const {
    return m_connectionWidget ? m_connectionWidget->address() : "";
}

quint16 ConnectionViewController::serverPort() const {
    return m_connectionWidget ? m_connectionWidget->serverPort() : 0;
}

quint16 ConnectionViewController::localPort() const {
    return m_connectionWidget ? m_connectionWidget->clientPort() : 0;
}

```

---


## Файл: ModbusClient/src/export/PngExportStrategy.cpp

```cpp
#include "PngExportStrategy.h"
#include <QPainter>
#include <QPixmap>

bool PngExportStrategy::exportWidget(QWidget* widget, const QString& filename) {
    if (!widget) {
        return false;
    }

    QPixmap pixmap(widget->size());
    widget->render(&pixmap);

    return pixmap.save(filename, "PNG");
}

```

---


## Файл: ModbusClient/src/export/interfaces/IExportStrategy.h

```cpp
#pragma once
#include <QWidget>
#include <QString>

class IExportStrategy {
public:
    virtual ~IExportStrategy() = default;
    virtual bool exportWidget(QWidget* widget, const QString& filename) = 0;
};

```

---


## Файл: ModbusClient/src/export/PngExportStrategy.h

```cpp
#pragma once
#include "interfaces/IExportStrategy.h"

class PngExportStrategy : public IExportStrategy {
public:
    bool exportWidget(QWidget* widget, const QString& filename) override;
};

```

---


## Файл: ModbusClient/src/main.cpp

```cpp

#include <QApplication>
#include <QScopedPointer>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QDebug>

#include "gui/mainwindow/MainWindow.h"
#include "gui/connection/ConnectionViewController.h"
#include "gui/database/DatabaseViewController.h"
#include "gui/monitoring/MonitoringViewController.h"
#include "gui/mode_selection/ModeSelectionViewController.h"

#include "core/modbus/DeltaModbusClient.h"
#include "core/connection/ConnectionManager.h"

#include "data/DataRepository.h"
#include "data/database/SqliteDatabaseRepository.h"
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/DatabaseExportService.h"

#include "monitoring/DataMonitor.h"

#include "control/ModeController.h"
#include "control/ControlStateMachine.h"
#include "control/ControlUIController.h"

#include "export/PngExportStrategy.h"

void setupLogging() {
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    qputenv("QT_MODBUS_TCP_TIMEOUT", "5000");
    qputenv("QT_MODBUS_TCP_RETRIES", "1");
}

void setupHighDPI() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

void setupCommandLine(QApplication& app, QCommandLineParser& parser) {
    parser.setApplicationDescription("Modbus TCP Client with GUI for Delta AS332T");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption noScalingOption("no-scaling", "Disable High DPI scaling");
    parser.addOption(noScalingOption);

    parser.process(app);

    if (parser.isSet(noScalingOption)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
#endif
    }
}

int main(int argc, char *argv[]) {
    setupHighDPI();

    QApplication app(argc, argv);
    app.setApplicationName("Modbus Client");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Красный Октябрь");
    app.setOrganizationDomain("koavia.ru");

    setupLogging();

    QCommandLineParser parser;
    setupCommandLine(app, parser);

    try {
        qDebug() << "=== Application Starting ===";

        // 1. Create core components
        qDebug() << "Creating Modbus client...";
        auto modbusClient = new DeltaModbusClient();
        auto exportStrategy = new PngExportStrategy();

        // 2. Create database components
        qDebug() << "Initializing database...";
        auto databaseRepository = new SqliteDatabaseRepository();
        if (!databaseRepository->initializeDatabase()) {
            qCritical() << "Failed to initialize database";
            return 1;
        }

        auto databaseManager = new DatabaseAsyncManager(databaseRepository);
        auto databaseExportService = new DatabaseExportService(databaseRepository);
        databaseManager->start();

        // 3. Create data repository with database support
        qDebug() << "Creating data repository...";
        auto dataRepository = new DataRepository(databaseManager);

        // 4. Create monitoring and control components
        qDebug() << "Creating connection and control components...";
        auto connectionManager = new ConnectionManager(modbusClient);
        auto dataMonitor = new DataMonitor(modbusClient, dataRepository);

        auto modeController = new ModeController(modbusClient);
        modeController->setDataRepository(dataRepository);
        qDebug() << "ModeController connected to DataRepository";

        // ВАЖНО: ControlStateMachine сам подключается к modbusClient для мониторинга статусов
        auto controlStateMachine = new ControlStateMachine(modbusClient);
        qDebug() << "ControlStateMachine initialized with status monitoring";

        // 5. Create view controllers
        qDebug() << "Creating view controllers...";
        auto connectionViewController = new ConnectionViewController(connectionManager);
        auto databaseViewController = new DatabaseViewController(databaseManager, databaseExportService);

        // MonitoringViewController больше НЕ нужен modbusClient
        auto monitoringViewController = new MonitoringViewController(
            dataMonitor,
            controlStateMachine,
            modeController,
            dataRepository
        );

        auto modeSelectionViewController = new ModeSelectionViewController(modeController);

        // 6. Setup connections between components
        qDebug() << "Setting up connections...";

        // Connect database signals
        QObject::connect(databaseManager, &DatabaseAsyncManager::testSessionsLoaded,
                        databaseViewController, &DatabaseViewController::showSessions);
        QObject::connect(databaseManager, &DatabaseAsyncManager::dataPointsLoaded,
                        databaseViewController, &DatabaseViewController::showSessionData);

        // Connect database view signals
        QObject::connect(databaseViewController, &DatabaseViewController::loadSessionsRequested,
                        databaseManager, &DatabaseAsyncManager::loadTestSessions);

        QObject::connect(databaseViewController, &DatabaseViewController::loadSessionDataRequested,
                        [databaseManager](int sessionId) {
                            databaseManager->loadDataPoints(sessionId, "");
                        });

        QObject::connect(databaseViewController, &DatabaseViewController::exportToCsvRequested,
                        databaseExportService, &DatabaseExportService::exportSessionToCsv);
        QObject::connect(databaseViewController, &DatabaseViewController::exportToImageRequested,
                        [databaseExportService, exportStrategy](int sessionId, const QString& filename) {
                            databaseExportService->exportSessionToImage(sessionId, filename, exportStrategy);
                        });

        // Connect connection status to mode selection
        QObject::connect(connectionManager, &ConnectionManager::connectionStatusChanged,
                        [modeSelectionViewController](const QString& status) {
                            bool connected = (status == "ПОДКЛЮЧЕН");
                            modeSelectionViewController->onConnectionStateChanged(connected);
                            qDebug() << "Connection state changed:" << connected;
                        });
        QObject::connect(connectionManager, &ConnectionManager::connectionStatusChanged,
                         [modeController](const QString& status) {
                             bool connected = (status == "ПОДКЛЮЧЕН");
                             modeController->setConnectionState(connected);
                             qDebug() << "ModeController connection state set to:" << connected;
                         });

        // Connect ControlStateMachine to ModeController for stopping tests
        QObject::connect(controlStateMachine, &ControlStateMachine::stopCurrentTest,
                        modeController, &ModeController::stopTest);

        // 7. Create and setup main window
        qDebug() << "Creating main window...";
        MainWindow mainWindow;
        mainWindow.setDataRepository(dataRepository);

        mainWindow.setupUI(connectionViewController, modeSelectionViewController,
                          monitoringViewController, databaseViewController);

        // 8. Start the application
        qDebug() << "Showing main window...";
        mainWindow.show();

        // Start monitoring
        qDebug() << "Starting monitoring...";
        monitoringViewController->startMonitoring();

        qDebug() << "=== Application Started Successfully ===";
        qDebug() << "✓ Status monitoring in ControlStateMachine";
        qDebug() << "✓ Automatic state transitions enabled";
        qDebug() << "✓ Clean architecture with proper separation of concerns";

        return app.exec();

    } catch (const std::exception& e) {
        qCritical() << "Fatal error:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Application failed to start: %1").arg(e.what()));
        return 1;
    }
}

```

---


## Файл: ModbusClient/src/data/DataPoint.h

```cpp
#pragma once
#include <QDateTime>
#include <QString>

struct DataPoint {
    QDateTime timestamp;
    QString parameter;
    double value;

    DataPoint() : value(0.0) {}

    DataPoint(const QString& param, double val)
        : timestamp(QDateTime::currentDateTime())
        , parameter(param)
        , value(val)
    {}
};

```

---


## Файл: ModbusClient/src/data/DataRepository.cpp

```cpp

#include "DataRepository.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <QDebug>

DataRepository::DataRepository(DatabaseAsyncManager* dbManager, QObject* parent)
    : IDataRepository(parent)
    , m_dbManager(dbManager)
    , m_sessionActive(false)
    , m_autoSaveTimer(new QTimer(this))
{
    if (m_dbManager) {
        connect(m_dbManager, SIGNAL(dataPointsSaved(int)),
                this, SLOT(onDataPointsSaved(int)));
        connect(m_dbManager, SIGNAL(dataPointsLoaded(QVector<DataPointRecord>)),
                this, SLOT(onDataPointsLoaded(QVector<DataPointRecord>)));
    }

    // Автосохранение каждые 30 секунд
    m_autoSaveTimer->setInterval(30000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &DataRepository::autoSave);

    qDebug() << "DataRepository: Initialized with auto-save every 30 seconds";
}

void DataRepository::addDataPoint(const QString& parameter, double value) {
    QWriteLocker locker(&m_lock);

    DataPoint point(parameter, value);
    m_data[parameter].append(point);

    locker.unlock();
    emit dataAdded(parameter, value);
}

void DataRepository::setCurrentTestSession(const QString& testType) {
    m_currentSession = TestSession(testType, QDateTime::currentDateTime());
    m_sessionActive = true;

    // Очищаем данные для новой сессии
    clearData();

    // Сразу создаём сессию в БД
    if (m_dbManager) {
        // Сохраняем сессию и получаем ID через сигнал
        connect(m_dbManager, &DatabaseAsyncManager::testSessionSaved,
                this, &DataRepository::onTestSessionSaved, Qt::UniqueConnection);
        m_dbManager->saveTestSession(m_currentSession);
        qDebug() << "DataRepository: Creating new test session in database:" << testType;
    }

    // Запускаем автосохранение
    if (m_autoSaveTimer && !m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->start();
        qDebug() << "DataRepository: Auto-save timer started";
    }
}

void DataRepository::onTestSessionSaved(int sessionId) {
    if (sessionId > 0) {
        m_currentSession.id = sessionId;
        qDebug() << "DataRepository: Test session saved with ID:" << sessionId;
        disconnect(m_dbManager, &DatabaseAsyncManager::testSessionSaved,
                   this, &DataRepository::onTestSessionSaved);

        emit sessionCreated(sessionId);
    } else {
        qCritical() << "DataRepository: Failed to get valid session ID";
    }
}

void DataRepository::saveCurrentSessionToDatabase() {
    if (!m_dbManager || !m_sessionActive) {
        qDebug() << "DataRepository: Cannot save - no active session or no database manager";
        return;
    }

    // Останавливаем автосохранение
    if (m_autoSaveTimer && m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->stop();
        qDebug() << "DataRepository: Auto-save timer stopped";
    }

    m_currentSession.endTime = QDateTime::currentDateTime();

    // Обновляем сессию в БД
    m_dbManager->saveTestSession(m_currentSession);

    // Сохраняем все накопленные данные
    saveToDatabaseAsync();

    qDebug() << "DataRepository: Test session saved to database. Duration:"
             << m_currentSession.startTime.secsTo(m_currentSession.endTime) << "seconds";
}

void DataRepository::finalizeSession() {
    if (m_sessionActive) {
        qDebug() << "DataRepository: Finalizing active session (emergency save)";
        saveCurrentSessionToDatabase();
        m_sessionActive = false;
    }
}

void DataRepository::saveToDatabaseAsync() {
    if (!m_dbManager || m_data.isEmpty() || m_currentSession.id <= 0) {
        qDebug() << "DataRepository: Nothing to save or no valid session ID";
        return;
    }

    QVector<DataPointRecord> points;
    QReadLocker locker(&m_lock);

    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        for (const auto& point : it.value()) {
            points.append(DataPointRecord(m_currentSession.id, it.key(), point.value, point.timestamp));
        }
    }

    locker.unlock();

    if (!points.isEmpty()) {
        m_dbManager->saveDataPoints(points);
        qDebug() << "DataRepository: Saving" << points.size() << "data points to database";
    }
}

void DataRepository::loadSessionFromDatabase(int sessionId) {
    if (m_dbManager) {
        // Загружаем все параметры для сессии
        m_dbManager->loadDataPoints(sessionId);
        qDebug() << "DataRepository: Loading session" << sessionId << "from database";
    }
}

QVector<TestSession> DataRepository::getHistoricalSessions(const QDateTime& from, const QDateTime& to, const QString& testType) {
    // Этот метод должен быть реализован через DatabaseAsyncManager
    // Пока возвращаем пустой список
    return QVector<TestSession>();
}

void DataRepository::onDataPointsSaved(int count) {
    qDebug() << "DataRepository: Successfully saved" << count << "data points to database";
    emit sessionSaved(m_currentSession.id);
}

void DataRepository::onDataPointsLoaded(const QVector<DataPointRecord>& points) {
    // Очищаем текущие данные
    clearData();

    // Загружаем исторические данные
    QWriteLocker locker(&m_lock);
    for (const auto& point : points) {
        DataPoint dataPoint(point.parameter, point.value);
        dataPoint.timestamp = point.timestamp;
        m_data[point.parameter].append(dataPoint);
    }
    locker.unlock();

    emit historicalDataLoaded(points);
    qDebug() << "DataRepository: Loaded" << points.size() << "historical data points";
}

QVector<DataPoint> DataRepository::getDataPoints(const QString& parameter,
                                                 const QDateTime& from,
                                                 const QDateTime& to) const {
    QReadLocker locker(&m_lock);

    if (!m_data.contains(parameter)) {
        return QVector<DataPoint>();
    }

    const auto& points = m_data[parameter];
    QVector<DataPoint> result;
    result.reserve(points.size());

    for (const auto& point : points) {
        if ((from.isNull() || point.timestamp >= from) &&
            (to.isNull() || point.timestamp <= to)) {
            result.append(point);
        }
    }

    return result;
}

QVector<QString> DataRepository::getAvailableParameters() const {
    QReadLocker locker(&m_lock);
    return m_data.keys().toVector();
}

void DataRepository::clearData(const QString& parameter) {
    QWriteLocker locker(&m_lock);

    if (parameter.isEmpty()) {
        m_data.clear();
        qDebug() << "DataRepository: All data cleared";
    } else {
        m_data.remove(parameter);
        qDebug() << "DataRepository: Data cleared for parameter:" << parameter;
    }

    locker.unlock();
    emit dataCleared(parameter);
}

int DataRepository::getDataPointCount(const QString& parameter) const {
    QReadLocker locker(&m_lock);

    if (parameter.isEmpty()) {
        int total = 0;
        for (const auto& vec : m_data) {
            total += vec.size();
        }
        return total;
    }

    return m_data.value(parameter).size();
}

void DataRepository::autoSave() {
    if (!m_sessionActive || !m_dbManager) {
        return;
    }

    // Периодически сохраняем накопленные данные (не завершая сессию)
    QVector<DataPointRecord> points;
    QReadLocker locker(&m_lock);

    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        for (const auto& point : it.value()) {
            points.append(DataPointRecord(m_currentSession.id, it.key(), point.value, point.timestamp));
        }
    }

    locker.unlock();

    if (!points.isEmpty()) {
        m_dbManager->saveDataPoints(points);
        qDebug() << "DataRepository: Auto-save triggered -" << points.size() << "points saved";
    }
}

```

---


## Файл: ModbusClient/src/data/DataRepository.h

```cpp

#pragma once
#include "interfaces/IDataRepository.h"
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/TestSession.h"
#include <QReadWriteLock>
#include <QMap>
#include <QVector>
#include <QObject>
#include <QTimer>

class DataRepository : public IDataRepository {
    Q_OBJECT
public:
    explicit DataRepository(DatabaseAsyncManager* dbManager = nullptr, QObject* parent = nullptr);
    ~DataRepository() override = default;

    void addDataPoint(const QString& parameter, double value) override;
    QVector<DataPoint> getDataPoints(const QString& parameter,
                                     const QDateTime& from = QDateTime(),
                                     const QDateTime& to = QDateTime()) const override;
    QVector<QString> getAvailableParameters() const override;
    void clearData(const QString& parameter = QString()) override;
    int getDataPointCount(const QString& parameter) const override;

    // Методы для работы с БД
    void setCurrentTestSession(const QString& testType) override;
    void saveCurrentSessionToDatabase() override;
    void finalizeSession() override; // Принудительное завершение сессии
    void loadSessionFromDatabase(int sessionId);
    QVector<TestSession> getHistoricalSessions(const QDateTime& from, const QDateTime& to, const QString& testType = "");

signals:
    void historicalDataLoaded(const QVector<DataPointRecord>& points);
    void sessionSaved(int sessionId);
    void sessionCreated(int sessionId);

private slots:
    void onDataPointsSaved(int count);
    void onDataPointsLoaded(const QVector<DataPointRecord>& points);
    void onTestSessionSaved(int sessionId);
    void autoSave(); // Автосохранение

private:
    mutable QReadWriteLock m_lock;
    QMap<QString, QVector<DataPoint>> m_data;
    DatabaseAsyncManager* m_dbManager;
    TestSession m_currentSession;
    bool m_sessionActive;
    QTimer* m_autoSaveTimer;

    void saveToDatabaseAsync();
};

```

---


## Файл: ModbusClient/src/data/database/DataPointDao.cpp

```cpp
#include "DataPointDao.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DataPointDao::DataPointDao(QSqlDatabase& database)
    : m_database(database)
{}

bool DataPointDao::createTable() {
    QSqlQuery query(m_database);
    return query.exec(
        "CREATE TABLE IF NOT EXISTS data_points ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "session_id INTEGER NOT NULL, "
        "parameter TEXT NOT NULL, "
        "value REAL NOT NULL, "
        "timestamp DATETIME NOT NULL, "
        "FOREIGN KEY(session_id) REFERENCES test_sessions(id) ON DELETE CASCADE"
        ")"
        );
}

bool DataPointDao::insertBatch(const QVector<DataPointRecord>& points) {
    if (points.isEmpty()) return true;

    m_database.transaction();

    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO data_points (session_id, parameter, value, timestamp) "
        "VALUES (:session_id, :parameter, :value, :timestamp)"
        );

    for (const auto& point : points) {
        query.bindValue(":session_id", point.sessionId);
        query.bindValue(":parameter", point.parameter);
        query.bindValue(":value", point.value);
        query.bindValue(":timestamp", point.timestamp);

        if (!query.exec()) {
            m_database.rollback();
            qWarning() << "Failed to insert data point:" << query.lastError().text();
            return false;
        }
    }

    return m_database.commit();
}

QVector<DataPointRecord> DataPointDao::findBySession(int sessionId, const QString& parameter) {
    QVector<DataPointRecord> points;
    QSqlQuery query(m_database);

    QString sql = "SELECT id, session_id, parameter, value, timestamp "
                  "FROM data_points WHERE session_id = :session_id";

    if (!parameter.isEmpty()) {
        sql += " AND parameter = :parameter";
    }
    sql += " ORDER BY timestamp ASC";

    query.prepare(sql);
    query.bindValue(":session_id", sessionId);
    if (!parameter.isEmpty()) {
        query.bindValue(":parameter", parameter);
    }

    if (query.exec()) {
        while (query.next()) {
            DataPointRecord point;
            point.id = query.value("id").toInt();
            point.sessionId = query.value("session_id").toInt();
            point.parameter = query.value("parameter").toString();
            point.value = query.value("value").toDouble();
            point.timestamp = query.value("timestamp").toDateTime();
            points.append(point);
        }
    }

    return points;
}

QVector<DataPointRecord> DataPointDao::findBySessionAndTimeRange(int sessionId,
                                                                 const QDateTime& from,
                                                                 const QDateTime& to,
                                                                 const QString& parameter) {
    QVector<DataPointRecord> points;
    QSqlQuery query(m_database);

    QString sql = "SELECT id, session_id, parameter, value, timestamp "
                  "FROM data_points WHERE session_id = :session_id "
                  "AND timestamp BETWEEN :from AND :to";

    if (!parameter.isEmpty()) {
        sql += " AND parameter = :parameter";
    }
    sql += " ORDER BY timestamp ASC";

    query.prepare(sql);
    query.bindValue(":session_id", sessionId);
    query.bindValue(":from", from);
    query.bindValue(":to", to);
    if (!parameter.isEmpty()) {
        query.bindValue(":parameter", parameter);
    }

    if (query.exec()) {
        while (query.next()) {
            DataPointRecord point;
            point.id = query.value("id").toInt();
            point.sessionId = query.value("session_id").toInt();
            point.parameter = query.value("parameter").toString();
            point.value = query.value("value").toDouble();
            point.timestamp = query.value("timestamp").toDateTime();
            points.append(point);
        }
    }

    return points;
}

int DataPointDao::getPointCount(int sessionId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM data_points WHERE session_id = :session_id");
    query.bindValue(":session_id", sessionId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

```

---


## Файл: ModbusClient/src/data/database/TestSessionDao.h

```cpp
#pragma once
#include "TestSession.h"
#include <QSqlDatabase>
#include <QVector>

class TestSessionDao {
public:
    explicit TestSessionDao(QSqlDatabase& database);

    bool createTable();
    int insert(const TestSession& session);
    bool update(const TestSession& session);
    QVector<TestSession> findSessions(const QDateTime& from,
                                      const QDateTime& to,
                                      const QString& testType = "");
    TestSession findById(int id);

private:
    QSqlDatabase& m_database;
};

```

---


## Файл: ModbusClient/src/data/database/SqliteDatabaseRepository.cpp

```cpp
#include "SqliteDatabaseRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

SqliteDatabaseRepository::SqliteDatabaseRepository(QObject* parent)
    : QObject(parent)
    , m_sessionDao(m_database)
    , m_dataPointDao(m_database)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_databasePath = dataDir + "/modbus_data.db";

    m_database = QSqlDatabase::addDatabase("QSQLITE", "modbus_connection");
    m_database.setDatabaseName(m_databasePath);
}

SqliteDatabaseRepository::SqliteDatabaseRepository(const QString& databasePath, QObject* parent)
    : QObject(parent)
    , m_sessionDao(m_database)
    , m_dataPointDao(m_database)
    , m_databasePath(databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", "modbus_connection");
    m_database.setDatabaseName(m_databasePath);
}

SqliteDatabaseRepository::~SqliteDatabaseRepository() {
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool SqliteDatabaseRepository::initializeDatabase() {
    if (!m_database.open()) {
        qCritical() << "Cannot open database:" << m_database.lastError().text();
        return false;
    }

    // Enable foreign keys
    QSqlQuery query(m_database);
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
    }

    // Create tables
    if (!m_sessionDao.createTable()) {
        qCritical() << "Failed to create test_sessions table";
        return false;
    }

    if (!m_dataPointDao.createTable()) {
        qCritical() << "Failed to create data_points table";
        return false;
    }

    qDebug() << "Database initialized successfully:" << m_databasePath;
    return true;
}

int SqliteDatabaseRepository::createTestSession(const TestSession& session) {
    return m_sessionDao.insert(session);
}

bool SqliteDatabaseRepository::updateTestSession(const TestSession& session) {
    return m_sessionDao.update(session);
}

QVector<TestSession> SqliteDatabaseRepository::getTestSessions(const QDateTime& from,
                                                               const QDateTime& to,
                                                               const QString& testType) {
    return m_sessionDao.findSessions(from, to, testType);
}

bool SqliteDatabaseRepository::saveDataPoints(const QVector<DataPointRecord>& points) {
    return m_dataPointDao.insertBatch(points);
}

QVector<DataPointRecord> SqliteDatabaseRepository::getDataPoints(int sessionId,
                                                                 const QString& parameter) {
    return m_dataPointDao.findBySession(sessionId, parameter);
}

QVector<DataPointRecord> SqliteDatabaseRepository::getDataPointsByTimeRange(int sessionId,
                                                                            const QDateTime& from,
                                                                            const QDateTime& to,
                                                                            const QString& parameter) {
    return m_dataPointDao.findBySessionAndTimeRange(sessionId, from, to, parameter);
}

int SqliteDatabaseRepository::getSessionCount() {
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM test_sessions") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

qint64 SqliteDatabaseRepository::getTotalDataPoints() {
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM data_points") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

```

---


## Файл: ModbusClient/src/data/database/DatabaseExportService.h

```cpp
#pragma once
#include "IDatabaseRepository.h"
#include "export/interfaces/IExportStrategy.h"
#include <QObject>

class DatabaseExportService : public QObject {
    Q_OBJECT
public:
    explicit DatabaseExportService(IDatabaseRepository* repository, QObject* parent = nullptr);

    void exportSessionToImage(int sessionId, const QString& filename, IExportStrategy* exportStrategy);
    void exportSessionToCsv(int sessionId, const QString& filename);

signals:
    void exportCompleted(const QString& filename);
    void exportFailed(const QString& error);

private:
    IDatabaseRepository* m_repository;
};

```

---


## Файл: ModbusClient/src/data/database/TestSessionDao.cpp

```cpp
#include "TestSessionDao.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

TestSessionDao::TestSessionDao(QSqlDatabase& database)
    : m_database(database)
{}

bool TestSessionDao::createTable() {
    QSqlQuery query(m_database);
    return query.exec(
        "CREATE TABLE IF NOT EXISTS test_sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "test_type TEXT NOT NULL, "
        "start_time DATETIME NOT NULL, "
        "end_time DATETIME, "
        "description TEXT"
        ")"
        );
}

int TestSessionDao::insert(const TestSession& session) {
    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO test_sessions (test_type, start_time, end_time, description) "
        "VALUES (:test_type, :start_time, :end_time, :description)"
        );
    query.bindValue(":test_type", session.testType);
    query.bindValue(":start_time", session.startTime);
    query.bindValue(":end_time", session.endTime);
    query.bindValue(":description", session.description);

    if (query.exec()) {
        return query.lastInsertId().toInt();
    } else {
        qWarning() << "Failed to insert test session:" << query.lastError().text();
        return -1;
    }
}

bool TestSessionDao::update(const TestSession& session) {
    QSqlQuery query(m_database);
    query.prepare(
        "UPDATE test_sessions SET "
        "test_type = :test_type, start_time = :start_time, "
        "end_time = :end_time, description = :description "
        "WHERE id = :id"
        );
    query.bindValue(":test_type", session.testType);
    query.bindValue(":start_time", session.startTime);
    query.bindValue(":end_time", session.endTime);
    query.bindValue(":description", session.description);
    query.bindValue(":id", session.id);

    return query.exec();
}

QVector<TestSession> TestSessionDao::findSessions(const QDateTime& from,
                                                  const QDateTime& to,
                                                  const QString& testType) {
    QVector<TestSession> sessions;
    QSqlQuery query(m_database);

    QString sql = "SELECT id, test_type, start_time, end_time, description "
                  "FROM test_sessions WHERE start_time BETWEEN :from AND :to";

    if (!testType.isEmpty()) {
        sql += " AND test_type = :test_type";
    }
    sql += " ORDER BY start_time DESC";

    query.prepare(sql);
    query.bindValue(":from", from);
    query.bindValue(":to", to);
    if (!testType.isEmpty()) {
        query.bindValue(":test_type", testType);
    }

    if (query.exec()) {
        while (query.next()) {
            TestSession session;
            session.id = query.value("id").toInt();
            session.testType = query.value("test_type").toString();
            session.startTime = query.value("start_time").toDateTime();
            session.endTime = query.value("end_time").toDateTime();
            session.description = query.value("description").toString();
            sessions.append(session);
        }
    }

    return sessions;
}

TestSession TestSessionDao::findById(int id) {
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM test_sessions WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        TestSession session;
        session.id = query.value("id").toInt();
        session.testType = query.value("test_type").toString();
        session.startTime = query.value("start_time").toDateTime();
        session.endTime = query.value("end_time").toDateTime();
        session.description = query.value("description").toString();
        return session;
    }

    return TestSession();
}

```

---


## Файл: ModbusClient/src/data/database/DatabaseAsyncManager.cpp

```cpp
#include "DatabaseAsyncManager.h"
#include <QMetaType>
#include <QDebug>

DatabaseAsyncManager::DatabaseAsyncManager(IDatabaseRepository* repository, QObject* parent)
    : QObject(parent)
    , m_repository(repository)
    , m_workerThread(new QThread(this))
    , m_running(false)
{
    qRegisterMetaType<QVector<TestSession>>("QVector<TestSession>");
    qRegisterMetaType<QVector<DataPointRecord>>("QVector<DataPointRecord>");

    moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, this, &DatabaseAsyncManager::processQueue);
}

DatabaseAsyncManager::~DatabaseAsyncManager() {
    stop();
}

void DatabaseAsyncManager::start() {
    if (!m_running) {
        m_running = true;
        m_workerThread->start();
        qDebug() << "Database async manager started";
    }
}

void DatabaseAsyncManager::stop() {
    if (m_running) {
        m_running = false;
        m_queueCondition.wakeAll();
        m_workerThread->quit();
        m_workerThread->wait(5000);
        qDebug() << "Database async manager stopped";
    }
}

void DatabaseAsyncManager::saveTestSession(const TestSession& session) {
    DatabaseOperation op;
    op.type = SaveSession;
    op.data.setValue(session);
    addOperation(op);
}

void DatabaseAsyncManager::saveDataPoints(const QVector<DataPointRecord>& points) {
    DatabaseOperation op;
    op.type = SaveDataPoints;
    op.data.setValue(points);
    addOperation(op);
}

void DatabaseAsyncManager::loadTestSessions(const QDateTime& from, const QDateTime& to, const QString& testType) {
    DatabaseOperation op;
    op.type = LoadSessions;
    op.data.setValue(QPair<QDateTime, QDateTime>(from, to));
    op.extra.setValue(testType);
    addOperation(op);
}

void DatabaseAsyncManager::loadDataPoints(int sessionId, const QString& parameter) {
    DatabaseOperation op;
    op.type = LoadDataPoints;
    op.data.setValue(sessionId);
    op.extra.setValue(parameter);
    addOperation(op);
}

void DatabaseAsyncManager::addOperation(const DatabaseOperation& operation) {
    QMutexLocker locker(&m_queueMutex);
    m_operationQueue.enqueue(operation);
    m_queueCondition.wakeOne();
}

void DatabaseAsyncManager::processQueue() {
    while (m_running) {
        DatabaseOperation operation;

        {
            QMutexLocker locker(&m_queueMutex);
            if (m_operationQueue.isEmpty()) {
                m_queueCondition.wait(&m_queueMutex);
                if (!m_running) break;
            }
            operation = m_operationQueue.dequeue();
        }

        try {
            switch (operation.type) {
            case SaveSession: {
                TestSession session = operation.data.value<TestSession>();
                int sessionId = m_repository->createTestSession(session);
                if (sessionId > 0) {
                    emit testSessionSaved(sessionId);
                } else {
                    emit errorOccurred("Failed to save test session");
                }
                break;
            }
            case SaveDataPoints: {
                QVector<DataPointRecord> points = operation.data.value<QVector<DataPointRecord>>();
                bool success = m_repository->saveDataPoints(points);
                if (success) {
                    emit dataPointsSaved(points.size());
                } else {
                    emit errorOccurred("Failed to save data points");
                }
                break;
            }
            case LoadSessions: {
                auto range = operation.data.value<QPair<QDateTime, QDateTime>>();
                QString testType = operation.extra.toString();
                QVector<TestSession> sessions = m_repository->getTestSessions(range.first, range.second, testType);
                emit testSessionsLoaded(sessions);
                break;
            }
            case LoadDataPoints: {
                int sessionId = operation.data.toInt();
                QString parameter = operation.extra.toString();
                QVector<DataPointRecord> points = m_repository->getDataPoints(sessionId, parameter);
                emit dataPointsLoaded(points);
                break;
            }
            }
        } catch (const std::exception& e) {
            emit errorOccurred(QString("Database operation failed: %1").arg(e.what()));
        }
    }
}

```

---


## Файл: ModbusClient/src/data/database/SqliteDatabaseRepository.h

```cpp
#pragma once
#include "IDatabaseRepository.h"
#include "TestSessionDao.h"
#include "DataPointDao.h"
#include <QSqlDatabase>
#include <QString>
#include <QObject>

class SqliteDatabaseRepository : public QObject, public IDatabaseRepository {
    Q_OBJECT
public:
    explicit SqliteDatabaseRepository(QObject* parent = nullptr);
    explicit SqliteDatabaseRepository(const QString& databasePath, QObject* parent = nullptr);
    ~SqliteDatabaseRepository() override;

    // IDatabaseRepository interface
    bool initializeDatabase() override;
    int createTestSession(const TestSession& session) override;
    bool updateTestSession(const TestSession& session) override;
    QVector<TestSession> getTestSessions(const QDateTime& from,
                                        const QDateTime& to,
                                        const QString& testType = "") override;

    bool saveDataPoints(const QVector<DataPointRecord>& points) override;
    QVector<DataPointRecord> getDataPoints(int sessionId,
                                          const QString& parameter = "") override;
    QVector<DataPointRecord> getDataPointsByTimeRange(int sessionId,
                                                     const QDateTime& from,
                                                     const QDateTime& to,
                                                     const QString& parameter = "") override;

    int getSessionCount() override;
    qint64 getTotalDataPoints() override;

private:
    QSqlDatabase m_database;
    TestSessionDao m_sessionDao;
    DataPointDao m_dataPointDao;
    QString m_databasePath;
};

```

---


## Файл: ModbusClient/src/data/database/TestSession.h

```cpp
#pragma once
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMetaType>

struct TestSession {
    int id;
    QString testType;
    QDateTime startTime;
    QDateTime endTime;
    QString description;

    TestSession() : id(-1) {}
    TestSession(const QString& type, const QDateTime& start)
        : id(-1), testType(type), startTime(start) {}
};

Q_DECLARE_METATYPE(TestSession)

struct DataPointRecord {
    int id;
    int sessionId;
    QString parameter;
    double value;
    QDateTime timestamp;

    DataPointRecord() : id(-1), sessionId(-1), value(0.0) {}
    DataPointRecord(int sessId, const QString& param, double val, const QDateTime& time)
        : id(-1), sessionId(sessId), parameter(param), value(val), timestamp(time) {}
};

```

---


## Файл: ModbusClient/src/data/database/DataPointDao.h

```cpp
#pragma once
#include "TestSession.h"
#include <QSqlDatabase>
#include <QVector>

class DataPointDao {
public:
    explicit DataPointDao(QSqlDatabase& database);

    bool createTable();
    bool insertBatch(const QVector<DataPointRecord>& points);
    QVector<DataPointRecord> findBySession(int sessionId, const QString& parameter = "");
    QVector<DataPointRecord> findBySessionAndTimeRange(int sessionId,
                                                       const QDateTime& from,
                                                       const QDateTime& to,
                                                       const QString& parameter = "");
    int getPointCount(int sessionId);

private:
    QSqlDatabase& m_database;
};

```

---


## Файл: ModbusClient/src/data/database/DatabaseExportService.cpp

```cpp
#include "DatabaseExportService.h"
#include "gui/widgets/ChartWidget.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

DatabaseExportService::DatabaseExportService(IDatabaseRepository* repository, QObject* parent)
    : QObject(parent)
    , m_repository(repository)
{}

void DatabaseExportService::exportSessionToImage(int sessionId, const QString& filename, IExportStrategy* exportStrategy) {
    try {
        // Загружаем данные сессии
        auto adPoints = m_repository->getDataPoints(sessionId, "AD_RPM");
        auto tkPoints = m_repository->getDataPoints(sessionId, "TK_RPM");
        auto stPoints = m_repository->getDataPoints(sessionId, "ST_RPM");

        if (adPoints.isEmpty() && tkPoints.isEmpty() && stPoints.isEmpty()) {
            emit exportFailed("No data available for export");
            return;
        }

        // Создаем временный виджет графика
        ChartWidget* chartWidget = new ChartWidget();

        // Добавляем данные в график
        // Здесь нужно добавить методы в ChartWidget для загрузки исторических данных
        // chartWidget->loadHistoricalData(adPoints, tkPoints, stPoints);

        // Экспортируем
        if (exportStrategy->exportWidget(chartWidget, filename)) {
            emit exportCompleted(filename);
        } else {
            emit exportFailed("Failed to export chart to image");
        }

        chartWidget->deleteLater();

    } catch (const std::exception& e) {
        emit exportFailed(QString("Export error: %1").arg(e.what()));
    }
}

void DatabaseExportService::exportSessionToCsv(int sessionId, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFailed("Cannot open file for writing");
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Заголовок CSV
    stream << "Timestamp,AD_RPM,TK_RPM,ST_RPM,TK_PERCENT,ST_PERCENT\n";

    try {
        // Загружаем все данные сессии
        auto adPoints = m_repository->getDataPoints(sessionId, "AD_RPM");
        auto tkPoints = m_repository->getDataPoints(sessionId, "TK_RPM");
        auto stPoints = m_repository->getDataPoints(sessionId, "ST_RPM");
        auto tkPercentPoints = m_repository->getDataPoints(sessionId, "TK_PERCENT");
        auto stPercentPoints = m_repository->getDataPoints(sessionId, "ST_PERCENT");

        // Собираем все временные метки
        QSet<QDateTime> allTimestamps;
        for (const auto& point : adPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : tkPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : stPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : tkPercentPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : stPercentPoints) allTimestamps.insert(point.timestamp);

        // Сортируем временные метки
        QList<QDateTime> sortedTimestamps = allTimestamps.values();
        std::sort(sortedTimestamps.begin(), sortedTimestamps.end());

        // Создаем карты для быстрого доступа к значениям по времени
        QMap<QDateTime, double> adMap, tkMap, stMap, tkPercentMap, stPercentMap;
        for (const auto& point : adPoints) adMap[point.timestamp] = point.value;
        for (const auto& point : tkPoints) tkMap[point.timestamp] = point.value;
        for (const auto& point : stPoints) stMap[point.timestamp] = point.value;
        for (const auto& point : tkPercentPoints) tkPercentMap[point.timestamp] = point.value;
        for (const auto& point : stPercentPoints) stPercentMap[point.timestamp] = point.value;

        // Записываем данные
        for (const auto& timestamp : sortedTimestamps) {
            stream << timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz") << ","
                   << adMap.value(timestamp, 0) << ","
                   << tkMap.value(timestamp, 0) << ","
                   << stMap.value(timestamp, 0) << ","
                   << tkPercentMap.value(timestamp, 0) << ","
                   << stPercentMap.value(timestamp, 0) << "\n";
        }

        file.close();
        emit exportCompleted(filename);

    } catch (const std::exception& e) {
        file.close();
        emit exportFailed(QString("CSV export error: %1").arg(e.what()));
    }
}

```

---


## Файл: ModbusClient/src/data/database/DatabaseAsyncManager.h

```cpp
#pragma once
#include "IDatabaseRepository.h"
#include "TestSession.h"
#include <QObject>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QVariant>
#include <QMutex>
#include <QWaitCondition>

// Регистрация метатипов для сигналов/слотов
Q_DECLARE_METATYPE(QVector<TestSession>)
Q_DECLARE_METATYPE(QVector<DataPointRecord>)

class DatabaseAsyncManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseAsyncManager(IDatabaseRepository* repository, QObject* parent = nullptr);
    ~DatabaseAsyncManager() override;

    void start();
    void stop();

    void saveTestSession(const TestSession& session);
    void saveDataPoints(const QVector<DataPointRecord>& points);
    void loadTestSessions(const QDateTime& from, const QDateTime& to, const QString& testType = "");
    void loadDataPoints(int sessionId, const QString& parameter = "");

signals:
    void testSessionSaved(int sessionId);
    void dataPointsSaved(int count);
    void testSessionsLoaded(const QVector<TestSession>& sessions);
    void dataPointsLoaded(const QVector<DataPointRecord>& points);
    void errorOccurred(const QString& error);

private slots:
    void processQueue();

private:
    enum OperationType {
        SaveSession,
        SaveDataPoints,
        LoadSessions,
        LoadDataPoints
    };

    struct DatabaseOperation {
        OperationType type;
        QVariant data;
        QVariant extra;
    };

    IDatabaseRepository* m_repository;
    QThread* m_workerThread;
    QQueue<DatabaseOperation> m_operationQueue;
    QMutex m_queueMutex;
    QWaitCondition m_queueCondition;
    bool m_running;

    void addOperation(const DatabaseOperation& operation);
};

```

---


## Файл: ModbusClient/src/data/database/IDatabaseRepository.h

```cpp
#pragma once
#include "TestSession.h"
#include <QVector>
#include <QDateTime>

class IDatabaseRepository {
public:
    virtual ~IDatabaseRepository() = default;

    // Session management
    virtual bool initializeDatabase() = 0;
    virtual int createTestSession(const TestSession& session) = 0;
    virtual bool updateTestSession(const TestSession& session) = 0;
    virtual QVector<TestSession> getTestSessions(const QDateTime& from,
                                                 const QDateTime& to,
                                                 const QString& testType = "") = 0;

    // Data points
    virtual bool saveDataPoints(const QVector<DataPointRecord>& points) = 0;
    virtual QVector<DataPointRecord> getDataPoints(int sessionId,
                                                   const QString& parameter = "") = 0;
    virtual QVector<DataPointRecord> getDataPointsByTimeRange(int sessionId,
                                                              const QDateTime& from,
                                                              const QDateTime& to,
                                                              const QString& parameter = "") = 0;

    // Statistics
    virtual int getSessionCount() = 0;
    virtual qint64 getTotalDataPoints() = 0;
};

```

---


## Файл: ModbusClient/src/data/interfaces/IDataRepository.h

```cpp
#pragma once
#include <QObject>
#include <QVector>
#include <QDateTime>
#include "../DataPoint.h"

class IDataRepository : public QObject {
    Q_OBJECT
public:
    explicit IDataRepository(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDataRepository() = default;

    virtual void addDataPoint(const QString& parameter, double value) = 0;
    virtual QVector<DataPoint> getDataPoints(const QString& parameter,
                                             const QDateTime& from = QDateTime(),
                                             const QDateTime& to = QDateTime()) const = 0;
    virtual QVector<QString> getAvailableParameters() const = 0;
    virtual void clearData(const QString& parameter = QString()) = 0;
    virtual int getDataPointCount(const QString& parameter) const = 0;
    virtual void setCurrentTestSession(const QString& testType) = 0;
    virtual void saveCurrentSessionToDatabase() = 0;
    virtual void finalizeSession() = 0;

signals:
    void dataAdded(const QString& parameter, double value);
    void dataCleared(const QString& parameter);
    void sessionCreated(int sessionId);
};

```

---


## Файл: ModbusClient/CMakeLists.txt

```cpp
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

```

---


## Файл: ModbusClient/tests/CMakeLists.txt

```cpp

```

---


## Файл: ModbusClient/tests/mocks/MockDataRepository.h

```cpp

```

---


## Файл: ModbusClient/tests/mocks/MockModbusClient.cpp

```cpp

```

---


## Файл: ModbusClient/tests/mocks/MockModbusClient.h

```cpp

```

---


## Файл: ModbusClient/tests/mocks/MockDataRepository.cpp

```cpp

```

---


## Файл: алгоритм.txt

```cpp
1)	Когда программа находится в окне выбора режима запуска, в регистр D0 записывается «0»;
2)	При нажатии на любую из пяти кнопок выбора режима работы, программа переходит в следующее окно (с сигналами, кнопками и графиками), а в регистр D0 записывается соответствующее режиму число:
a.	Расконсервация/Консервация – «1»
b.	Хол. прокр. турбостартера – «2»
c.	Рег. мощности/замер парам. – «3»
d.	Хол. прокр. основного двиг. – «4»
e.	Имитация зап. осн. двиг. – «5»
3)	В появившемся окне управления экранные кнопки при нажатии на них записывают в соответствующие им регистры «1», а при отпускании – «0»:
a.	«Проверка готовности» – регистр M1
b.	«ПУСК» – регистр M2
c.	«СТОП» – регистр M3
d.	«Повторение запуска» – регистр M4
e.	«Прерывание запуска» – регистр M5
f.	«ВЫХОД» – регистр M6
4)	В окне управления запуском сразу после перехода в него доступны только кнопки «Проверка готовности» и «ВЫХОД»;
5)	После нажатия кнопки «Проверка готовности» и при выполнении всех условий, необходимых для запуска в выбранном режиме работы турбостартера, регистр M11 становится «1»;
6)	Если регистр M11 становится «1» кнопки  «Проверка готовности» и «ВЫХОД» пропадают с экрана, а кнопки «ПУСК» и «Прерывание запуска» появляются на экране;
7)	После нажатия кнопки «ПУСК» регистр M12 становится «1», кнопки «ПУСК» и «Прерывание запуска» пропадают с экрана, а кнопка «СТОП» появляется на экране;
8)	После штатного окончания запуска (в соответствии с алгоритмом программы) или прерывания запуска путем нажатия кнопки «СТОП» и остановки всех вращающихся частей (электропривод, турбокомпрессор и свободная турбина) регистры M0 и M14 становятся «1», кнопка «СТОП» пропадает с экрана, а кнопки «Повторение запуска» и «ВЫХОД» появляются на экране (далее, соответственно, два варианта развития событий);
9)	После нажатия кнопки «Повторение запуска» состояние экранных кнопок возвращается к описанному в пункте 4); После нажатия кнопки «ВЫХОД» в регистр D0 записывается «0» и происходит возврат в окно выбора режима запуска;

```

---


