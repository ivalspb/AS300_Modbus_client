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
