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

