#pragma once
#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QModbusDataUnit>


class IModbusClient;

class ControlStateMachine : public QObject {
    Q_OBJECT
public:
    enum State {
        STATE_READY_CHECK,    // Состояние 1: Проверка готовности
        STATE_START_INTERRUPT,// Состояние 2: Пуск/Прерывание
        STATE_STOP,           // Состояние 3: Стоп
        STATE_RESTART_EXIT    // Состояние 4: Повторение/Выход
    };
    Q_ENUM(State)  // Добавляем для регистрации

    explicit ControlStateMachine(IModbusClient* client, QObject* parent = nullptr);
    ~ControlStateMachine();

    State currentState() const { return m_currentState; }
    static void registerMetaTypes();

signals:
    void stateChanged(ControlStateMachine::State newState);
    void logMessage(const QString& message);
    void readyCheckRequested();
    void startRequested();
    void stopRequested();
    void restartRequested();
    void interruptRequested();
    void exitRequested();
    void modeSelectionRequested(); // Сигнал для переключения на вкладку выбора режима
    void registerWriteVerified(const QString& registerName, quint16 value, bool success);
    void stopCurrentTest();

    void startChartRecording();  // Сигнал для начала записи графика
    void stopChartRecording();   // Сигнал для остановки записи графика

    // Сигналы для переходов state machine
    void m11ReadySignal();
    void m12StartedSignal();
    void completionSignal();
    void startTriggered();
    void stopTriggered();
    void restartTriggered();
    void interruptTriggered();
    void exitTriggered();

public slots:
    void triggerReadyCheck();
    void triggerStart();
    void triggerStop();
    void triggerRestart();
    void triggerInterrupt();
    void triggerExit();

    // Слоты для обработки статусов от устройства
    void onM11StatusChanged(bool ready);
    void onM12StatusChanged(bool started);
    void onM0M14StatusChanged(bool completed);

private slots:
    void transitionToReadyCheck();
    void transitionToStartInterrupt();
    void transitionToStop();
    void transitionToRestartExit();

private:
    void setupStateMachine();
    void writeM1ReadyCheck();
    void writeM2Start();
    void writeM3Stop();
    void writeM4Restart();
    void writeM5Interrupt();
    void writeM6Exit();

    void writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address,
                                   quint16 value, const QString& registerName);
    void logRegisterOperation(const QString& operation, quint16 address,
                                 quint16 value, const QString& registerName, bool success = true);

    IModbusClient* m_client;
    State m_currentState;
    QStateMachine* m_stateMachine;
    QState* m_readyCheckState;
    QState* m_startInterruptState;
    QState* m_stopState;
    QState* m_restartExitState;

    bool m_m11Ready;
    bool m_m12Started;
    bool m_completionStatus;
};
