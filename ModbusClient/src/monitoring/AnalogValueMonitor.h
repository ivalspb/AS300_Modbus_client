#pragma once
#include <QObject>
#include <QModbusDataUnit>
#include "core/interfaces/IIndicator.h"

class IModbusClient;
class IAddressMapper;

class AnalogValueMonitor : public QObject {
    Q_OBJECT
public:
    explicit AnalogValueMonitor(IModbusClient* client,
                                IAddressMapper* mapper,
                                QObject* parent = nullptr);

    void setIndicators(IDualIndicator* adIndicator,
                       IDualIndicator* tkIndicator,
                       IDualIndicator* stIndicator);

    void startMonitoring();
    void stopMonitoring();

signals:
    void valueChanged(int analog, double value);

private slots:
    void onRegisterRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);
    void onRegistersRead(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values);

private:
    double convertValue(int analog, quint16 rawValue);
    double convertDWORDValue(int analog, const QVector<quint16>& values);

    IModbusClient* m_client;
    IAddressMapper* m_mapper;
    IDualIndicator* m_adIndicator;
    IDualIndicator* m_tkIndicator;
    IDualIndicator* m_stIndicator;
};
