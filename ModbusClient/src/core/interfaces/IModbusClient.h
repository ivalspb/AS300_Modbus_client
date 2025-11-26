#pragma once
#include <QObject>
#include <QVector>
#include <QVariant>
#include <QModbusDataUnit>
#include <QModbusDevice>

class IModbusClient : public QObject {
    Q_OBJECT
public:
    explicit IModbusClient(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IModbusClient() = default;

    virtual void setConnectionParameter(QModbusDevice::ConnectionParameter param, const QVariant& value) = 0;

    // Connection management
    virtual bool connectToDevice(const QString& address, quint16 port) = 0;
    virtual void disconnectFromDevice() = 0;
    virtual bool isConnected() const = 0;

    // New unified interface with register type
    virtual void readRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 count = 1) = 0;
    virtual void writeRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) = 0;

    virtual void writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, int timeoutMs = 3000)=0;

    enum PollingFrequency {
        HighFrequency = 20,  // 20 Hz
        LowFrequency = 2     // 2 Hz
    };

    // virtual void setPollingInterval(int intervalMs) = 0;
    // virtual void addPolledRegister(const QString& name, QModbusDataUnit::RegisterType type, quint16 address, quint16 count = 1) = 0;

    // Polling configuration with register type
    virtual void addPolledRegisterWithFrequency(const QString& name,
                                                QModbusDataUnit::RegisterType type,
                                                quint16 address,
                                                quint16 count = 1,
                                                PollingFrequency frequency = LowFrequency) = 0;
    virtual void removePolledRegister(const QString& name) = 0;
    virtual void clearPolledRegisters() = 0;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString& error);
    void dataRead(const QString& parameter, const QVariant& value);
    void registerReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, quint16 value);
    void registersReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values);
    void registerWriteCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success);
    void registerWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success);
};
