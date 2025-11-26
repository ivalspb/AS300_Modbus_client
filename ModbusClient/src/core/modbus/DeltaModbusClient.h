#pragma once
#include "core/interfaces/IModbusClient.h"
#include "core/interfaces/IRequestQueue.h"
#include "core/interfaces/IAddressMapper.h"
#include <QTimer>
#include <QModbusDevice>
#include <QScopedPointer>
#include <QDateTime>

class CustomModbusClient;
class ModbusRequestHandler;

class DeltaModbusClient : public IModbusClient {
    Q_OBJECT
public:
    explicit DeltaModbusClient(QObject* parent = nullptr);
    ~DeltaModbusClient() override;

    void setConnectionParameter(QModbusDevice::ConnectionParameter param, const QVariant& value) override;

    // IModbusClient interface
    bool connectToDevice(const QString& address, quint16 port) override;
    void disconnectFromDevice() override;
    bool isConnected() const override;

    // New unified methods with register type
    void readRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 count = 1) override;
    void writeRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) override;

    // Verification method
    void writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, int timeoutMs = 3000) override;

    // Polling configuration
    // void setPollingInterval(int intervalMs) override;
    void removePolledRegister(const QString& name) override;
    void clearPolledRegisters() override;

    void addPolledRegisterWithFrequency(const QString& name,
                                        QModbusDataUnit::RegisterType type,
                                        quint16 address,
                                        quint16 count = 1,
                                        PollingFrequency frequency = LowFrequency) override;

    // Delta-specific methods
    void setLocalPort(quint16 port);
    void setOperationMode(const QString& mode);
    QString currentOperationMode() const { return m_currentMode; }

private slots:
    void onStateChanged(QModbusDevice::State state);
    void onErrorOccurred(QModbusDevice::Error error);
    // void onPollTimeout();
    void onReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, const QString& paramName);
    void onReadsCompleted(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values);
    void onWriteCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success);
    void onRequestFailed(const QString& error);
    void onVerificationTimeout();

private:
    // Polling groups
    enum PollingGroup {
        HIGH_FREQUENCY,  // 20 Hz - analog channels
        LOW_FREQUENCY    // 2 Hz - all others
    };

    void initializeD0();
    void setupPollingTimers();
    void setupPollingGroups();
    void pollGroup(PollingGroup group);

    struct PolledRegister {
        QModbusDataUnit::RegisterType type;
        quint16 address;
        quint16 count;

        PolledRegister() : type(QModbusDataUnit::HoldingRegisters), address(0) {}
        PolledRegister(QModbusDataUnit::RegisterType t, quint16 addr, quint16 cnt = 1) : type(t), address(addr), count(cnt) {}
    };

    struct VerificationRequest {
        QModbusDataUnit::RegisterType type;
        quint16 address;
        quint16 expectedValue;
        QDateTime timestamp;
    };

    QMap<PollingGroup, QTimer*> m_pollTimers;
    QMap<PollingGroup, QMap<QString, PolledRegister>> m_pollingGroups;

    QScopedPointer<CustomModbusClient> m_client;
    QScopedPointer<IRequestQueue> m_queue;
    QScopedPointer<ModbusRequestHandler> m_handler;
    QScopedPointer<IAddressMapper> m_mapper;

    QTimer* m_verificationTimer;
    // QMap<QString, PolledRegister> m_polledRegisters;
    QMap<quint16, VerificationRequest> m_verificationRequests;
    QString m_currentMode;
    quint16 m_localPort;
    bool m_initialD0Written;
};
