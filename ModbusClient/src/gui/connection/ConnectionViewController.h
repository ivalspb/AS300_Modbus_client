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
