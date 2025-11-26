#pragma once
#include <QObject>
#include <QTimer>

class IModbusClient;
class IDataRepository;
class DiscreteInputMonitor;
class AnalogValueMonitor;

class DataMonitor : public QObject {
    Q_OBJECT
public:
    explicit DataMonitor(IModbusClient* client,
                         IDataRepository* repository,
                         QObject* parent = nullptr);
    ~DataMonitor() override;

    DiscreteInputMonitor* discreteMonitor() const { return m_discreteMonitor; }
    AnalogValueMonitor* analogMonitor() const { return m_analogMonitor; }

    void startMonitoring();
    void stopMonitoring();
    void setUpdateInterval(int ms);

signals:
    void dataUpdated(const QString& parameter, double value);
    void logMessage(const QString& message);

private slots:
    void updateData();
    void onDiscreteStatusChanged(int input, bool value);
    void onAnalogValueChanged(int analog, double value);
    void onCommandChanged(int output, bool value);

private:
    IModbusClient* m_client;
    IDataRepository* m_repository;
    DiscreteInputMonitor* m_discreteMonitor;
    AnalogValueMonitor* m_analogMonitor;
    QTimer* m_updateTimer;
};

