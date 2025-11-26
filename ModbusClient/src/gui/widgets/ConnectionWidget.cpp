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

