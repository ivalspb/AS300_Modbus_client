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
