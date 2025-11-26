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
