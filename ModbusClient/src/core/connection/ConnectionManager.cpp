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

