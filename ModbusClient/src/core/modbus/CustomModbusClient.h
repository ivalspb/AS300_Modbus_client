#pragma once
#include <QModbusTcpClient>
#include <QTcpSocket>

class CustomModbusClient : public QModbusTcpClient {
    Q_OBJECT
public:
    explicit CustomModbusClient(QObject* parent = nullptr);
    bool bindToPort(quint16 port);

private:
    QTcpSocket* m_customSocket;
};

