#pragma once
#include <QObject>
#include <QMap>
#include <QModbusDataUnit>

class IModbusClient;
class QPushButton;

class ModeController : public QObject {
    Q_OBJECT
public:
    explicit ModeController(IModbusClient* client, QObject* parent = nullptr);

    void setModeButtons(QPushButton* mode1, QPushButton* mode2, QPushButton* mode3,
                       QPushButton* mode4, QPushButton* mode5, QPushButton* stopButton);
    void setConnectionState(bool connected);
    QString currentMode() const { return m_currentMode; }

signals:
    void modeChanged(const QString& mode);
    void testStarted(const QString& mode);
    void testStopped();
    void logMessage(const QString& message);

public slots:
    void stopTest();  // Принудительная остановка теста

private slots:
    void onMode1Clicked();
    void onMode2Clicked();
    void onMode3Clicked();
    void onMode4Clicked();
    void onMode5Clicked();
    void onStopClicked();
    void onRegisterWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success);

private:
    void startTest(const QString& mode, QPushButton* button);
    void initializeModes();
    void updateButtonStates();
    void setActiveModeButton(QPushButton* activeButton);

    IModbusClient* m_client;
    QPushButton* m_modeButton1;
    QPushButton* m_modeButton2;
    QPushButton* m_modeButton3;
    QPushButton* m_modeButton4;
    QPushButton* m_modeButton5;
    QPushButton* m_stopButton;
    QMap<QString, QString> m_modeMap;
    QString m_currentMode;
    bool m_connected;
    bool m_testRunning;
};
