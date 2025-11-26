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
