#pragma once
#include <QObject>
#include <QMap>
#include <QModbusDataUnit>

class IModbusClient;
class QPushButton;
class IDataRepository;

class ModeController : public QObject {
    Q_OBJECT
public:
    explicit ModeController(IModbusClient* client, QObject* parent = nullptr);

    void setConnectionState(bool connected);
    void setDataRepository(IDataRepository* repository) { m_dataRepository = repository; }
    QString currentMode() const { return m_currentMode; }

signals:
    void modeChanged(const QString& mode);
    void testStarted(const QString& mode);
    void testStopped();
    void logMessage(const QString& message);

public slots:
    void stopTest();  // Принудительная остановка теста
    void onMode1Clicked();
    void onMode2Clicked();
    void onMode3Clicked();
    void onMode4Clicked();
    void onMode5Clicked();
    void onStopClicked();
    void onRegisterWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success);
    void onSessionCreated(int sessionId);

private:
    void startTest(const QString& mode);
    void initializeModes();
    void writeModeRegister();

    IModbusClient* m_client;
    IDataRepository* m_dataRepository;
    QString m_currentMode;
    bool m_connected;
    bool m_testRunning;
};
