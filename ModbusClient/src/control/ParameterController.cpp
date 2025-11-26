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
