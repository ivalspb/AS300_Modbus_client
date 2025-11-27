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
