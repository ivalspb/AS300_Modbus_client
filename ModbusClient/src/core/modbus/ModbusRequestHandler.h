#pragma once
#include <QObject>
#include <QModbusTcpClient>
#include <QTimer>
#include <QModbusReply>

class IRequestQueue;

class ModbusRequestHandler : public QObject {
    Q_OBJECT
public:
    explicit ModbusRequestHandler(QModbusTcpClient* client,
                                  IRequestQueue* queue,
                                  QObject* parent = nullptr);
    ~ModbusRequestHandler() override;

    void setProcessInterval(int ms);
    void start();
    void stop();
    bool isProcessing() const { return m_processing; }

signals:
    void readCompleted(QModbusDataUnit::RegisterType type, quint16 address,
                      quint16 value, const QString& paramName);
    void readsCompleted(QModbusDataUnit::RegisterType type, quint16 address,
                       const QVector<quint16>& values);
    void writeCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success);
    void requestFailed(const QString& error);

private slots:
    void processNextRequest();
    void handleReadReply();
    void handleWriteReply();

private:
    void sendReadRequest(QModbusDataUnit::RegisterType type, quint16 address,
                        quint16 count, const QString& paramName);
    void sendWriteRequest(QModbusDataUnit::RegisterType type, quint16 address,
                         quint16 value);

    QModbusTcpClient* m_client;
    IRequestQueue* m_queue;
    QTimer* m_processTimer;
    bool m_processing;
    int m_minRequestInterval;
};
