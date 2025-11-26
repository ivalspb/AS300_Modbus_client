#pragma once
#include <QObject>
#include <QLabel>
#include <QVector>
#include <QMap>
#include <QModbusDataUnit>

class IModbusClient;
class IAddressMapper;

class DiscreteInputMonitor : public QObject {
    Q_OBJECT
public:
    explicit DiscreteInputMonitor(IModbusClient* client,
                                  IAddressMapper* mapper,
                                  QObject* parent = nullptr);

    void setDiscreteLabels(const QVector<QLabel*>& labels);
    void setCommandLabels(const QVector<QLabel*>& labels);
    void startMonitoring();
    void stopMonitoring();

signals:
    void statusChanged(int input, bool value);
    void commandChanged(int output, bool value);

private slots:
    void onRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);

private:
    void updateIndicator(QLabel* label, bool value);
    void updateDiscreteIndicator(int input, bool value);
    void updateCommandIndicator(int output, bool value);

    IModbusClient* m_client;
    IAddressMapper* m_mapper;
    QVector<QLabel*> m_discreteLabels;
    QVector<QLabel*> m_commandLabels;
    QMap<quint16, int> m_addressToDiscreteMap;
    QMap<quint16, int> m_addressToCommandMap;
};
