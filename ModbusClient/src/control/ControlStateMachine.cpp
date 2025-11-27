#include "ControlStateMachine.h"
#include "core/interfaces/IModbusClient.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QTimer>
#include <QDateTime>
#include <QFinalState>
#include <QSignalTransition>
#include <QDebug>


ControlStateMachine::ControlStateMachine(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_currentState(STATE_READY_CHECK)
    , m_stateMachine(new QStateMachine(this))
    , m_m11Ready(false)
    , m_m12Started(false)
    , m_m0Status(false)
    , m_m14Status(false)
{
    registerMetaTypes();
    setupStateMachine();
    setupStatusMonitoring(); // НОВЫЙ метод
}

ControlStateMachine::~ControlStateMachine() {
    if (m_stateMachine) {
        m_stateMachine->stop();
    }
}

void ControlStateMachine::registerMetaTypes() {
    qRegisterMetaType<ControlStateMachine::State>("ControlStateMachine::State");
}

void ControlStateMachine::setupStatusMonitoring() {
    if (!m_client) {
        qCritical() << "ControlStateMachine: IModbusClient is null!";
        return;
    }

    // КРИТИЧЕСКИ ВАЖНО: Подключаемся к сигналу чтения регистров
    connect(m_client, &IModbusClient::registerReadCompleted,
            this, &ControlStateMachine::onStatusRegisterRead);

    qDebug() << "ControlStateMachine: Status register monitoring configured";
}

void ControlStateMachine::onStatusRegisterRead(QModbusDataUnit::RegisterType type,
                                               quint16 address,
                                               quint16 value) {
    // Обрабатываем только Coils (M-регистры)
    if (type != QModbusDataUnit::Coils) {
        return;
    }

    // M11 - готовность к запуску
    if (address == DeltaAS332T::Addresses::M11_READY_STATUS) {
        bool ready = (value > 0);
        if (m_m11Ready != ready) {
            m_m11Ready = ready;
            if (ready && m_currentState == STATE_READY_CHECK) {
                qDebug() << "ControlStateMachine: M11=1 detected, transitioning to START_INTERRUPT";
                emit m11ReadySignal();
            }
        }
    }
    // M12 - тест запущен
    else if (address == DeltaAS332T::Addresses::M12_START_STATUS) {
        bool started = (value > 0);
        if (m_m12Started != started) {
            m_m12Started = started;
            if (started && m_currentState == STATE_START_INTERRUPT) {
                qDebug() << "ControlStateMachine: M12=1 detected, transitioning to STOP";
                emit m12StartedSignal();
            }
        }
    }
    // M0 - остановка
    else if (address == DeltaAS332T::Addresses::M0_STOP_STATUS) {
        bool stopped = (value > 0);
        if (m_m0Status != stopped) {
            m_m0Status = stopped;
            qDebug() << "ControlStateMachine: M0 changed to:" << stopped;
            checkCompletionCondition();
        }
    }
    // M14 - завершение
    else if (address == DeltaAS332T::Addresses::M14_COMPLETE_STATUS) {
        bool completed = (value > 0);
        if (m_m14Status != completed) {
            m_m14Status = completed;
            qDebug() << "ControlStateMachine: M14 changed to:" << completed;
            checkCompletionCondition();
        }
    }
}

void ControlStateMachine::checkCompletionCondition() {
    // Проверяем условие завершения только в состоянии STOP
    if (m_currentState == STATE_STOP) {
        bool completionCondition = m_m0Status && m_m14Status;

        if (completionCondition) {
            qDebug() << "ControlStateMachine: Completion condition met (M0=1 && M14=1)";
            emit completionSignal();
        }
    }
}

void ControlStateMachine::setupStateMachine() {
    // Создаем состояния
    m_readyCheckState = new QState();
    m_startInterruptState = new QState();
    m_stopState = new QState();
    m_restartExitState = new QState();

    // НАСТРАИВАЕМ ПРАВИЛЬНЫЕ ПЕРЕХОДЫ:

    // 1. STATE_READY_CHECK -> STATE_START_INTERRUPT (при M11 == 1)
    QSignalTransition* toStartInterruptTransition = new QSignalTransition(this, &ControlStateMachine::m11ReadySignal);
    toStartInterruptTransition->setTargetState(m_startInterruptState);
    m_readyCheckState->addTransition(toStartInterruptTransition);

    // 2. STATE_START_INTERRUPT -> STATE_STOP (при M12 == 1)
    QSignalTransition* toStopTransition = new QSignalTransition(this, &ControlStateMachine::m12StartedSignal);
    toStopTransition->setTargetState(m_stopState);
    m_startInterruptState->addTransition(toStopTransition);

    // 3. STATE_STOP -> STATE_RESTART_EXIT (при M0 == 1 && M14 == 1) - АВТОМАТИЧЕСКИЙ ПЕРЕХОД
    QSignalTransition* toRestartExitTransition = new QSignalTransition(this, &ControlStateMachine::completionSignal);
    toRestartExitTransition->setTargetState(m_restartExitState);
    m_stopState->addTransition(toRestartExitTransition);

    // 4. STATE_START_INTERRUPT -> STATE_READY_CHECK (при нажатии Прерывание)
    QSignalTransition* interruptTransition = new QSignalTransition(this, &ControlStateMachine::interruptTriggered);
    interruptTransition->setTargetState(m_readyCheckState);
    m_startInterruptState->addTransition(interruptTransition);

    // 5. STATE_STOP -> STATE_RESTART_EXIT при нажатии Стоп
    QSignalTransition* stopToRestartTransition = new QSignalTransition(this, &ControlStateMachine::stopTriggered);
    stopToRestartTransition->setTargetState(m_restartExitState);
    m_stopState->addTransition(stopToRestartTransition);

    // 6. STATE_RESTART_EXIT -> STATE_READY_CHECK (при нажатии Повторение запуска)
    QSignalTransition* restartTransition = new QSignalTransition(this, &ControlStateMachine::restartTriggered);
    restartTransition->setTargetState(m_readyCheckState);
    m_restartExitState->addTransition(restartTransition);

    // 7. STATE_READY_CHECK -> STATE_READY_CHECK (при нажатии Выход)
    QSignalTransition* exitFromReadyTransition = new QSignalTransition(this, &ControlStateMachine::exitTriggered);
    exitFromReadyTransition->setTargetState(m_readyCheckState);
    m_readyCheckState->addTransition(exitFromReadyTransition);

    // 8. STATE_RESTART_EXIT -> STATE_READY_CHECK (при нажатии Выход)
    QSignalTransition* exitFromRestartTransition = new QSignalTransition(this, &ControlStateMachine::exitTriggered);
    exitFromRestartTransition->setTargetState(m_readyCheckState);
    m_restartExitState->addTransition(exitFromRestartTransition);

    // Подключаем обработчики входа в состояния
    connect(m_readyCheckState, &QState::entered, this, &ControlStateMachine::transitionToReadyCheck);
    connect(m_startInterruptState, &QState::entered, this, &ControlStateMachine::transitionToStartInterrupt);
    connect(m_stopState, &QState::entered, this, &ControlStateMachine::transitionToStop);
    connect(m_restartExitState, &QState::entered, this, &ControlStateMachine::transitionToRestartExit);

    // Добавляем состояния в машину
    m_stateMachine->addState(m_readyCheckState);
    m_stateMachine->addState(m_startInterruptState);
    m_stateMachine->addState(m_stopState);
    m_stateMachine->addState(m_restartExitState);

    // Устанавливаем начальное состояние
    m_stateMachine->setInitialState(m_readyCheckState);
    m_stateMachine->start();

    qDebug() << "ControlStateMachine: State machine initialized and started";
}

void ControlStateMachine::transitionToReadyCheck() {
    m_currentState = STATE_READY_CHECK;

    // Сбрасываем статусы при возврате в начальное состояние
    m_m11Ready = false;
    m_m12Started = false;
    m_m0Status = false;
    m_m14Status = false;

    resetStatusRegisters();

    emit stateChanged(STATE_READY_CHECK);
    emit logMessage("Состояние: Проверка готовности");

    qDebug() << "ControlStateMachine: Transitioned to READY_CHECK";
}

void ControlStateMachine::transitionToStartInterrupt() {
    m_currentState = STATE_START_INTERRUPT;
    emit stateChanged(STATE_START_INTERRUPT);
    emit logMessage("Состояние: Пуск/Прерывание");

    qDebug() << "ControlStateMachine: Transitioned to START_INTERRUPT";
}

void ControlStateMachine::transitionToStop() {
    m_currentState = STATE_STOP;
    emit stateChanged(STATE_STOP);
    emit logMessage("Состояние: Стоп");

    qDebug() << "ControlStateMachine: Transitioned to STOP";
}

void ControlStateMachine::transitionToRestartExit() {
    m_currentState = STATE_RESTART_EXIT;
    emit stateChanged(STATE_RESTART_EXIT);

    // ОСТАНАВЛИВАЕМ ЗАПИСЬ ГРАФИКА ПРИ ПЕРЕХОДЕ В СОСТОЯНИЕ RESTART_EXIT
    emit stopChartRecording();
    emit logMessage("Состояние: Повторение/Выход - остановка записи графика");

    qDebug() << "ControlStateMachine: Transitioned to RESTART_EXIT";
}

// Public slots для внешних триггеров
void ControlStateMachine::triggerReadyCheck() {
    if (m_currentState == STATE_READY_CHECK) {
        writeM1ReadyCheck();
        emit readyCheckRequested();
    }
}

void ControlStateMachine::triggerStart() {
    if (m_currentState == STATE_START_INTERRUPT) {
        writeM2Start();
        emit startRequested();
        emit startTriggered(); // Сигнал для state machine
        // ЗАПУСКАЕМ ЗАПИСЬ ГРАФИКА ПРИ НАЖАТИИ ПУСК
        emit startChartRecording();
        emit logMessage("Запуск процесса - начало записи графика");
    }
}

void ControlStateMachine::triggerStop() {
    if (m_currentState == STATE_STOP) {
        writeM3Stop();
        emit stopRequested();
        emit stopTriggered(); // Сигнал для state machine
        emit logMessage("Ручная остановка - переход в состояние Повторение/Выход");
    }
}

void ControlStateMachine::triggerRestart() {
    if (m_currentState == STATE_RESTART_EXIT) {
        writeM4Restart();
        emit restartRequested();
        emit restartTriggered(); // Сигнал для state machine
    }
}

void ControlStateMachine::triggerInterrupt() {
    if (m_currentState == STATE_START_INTERRUPT) {
        writeM5Interrupt();
        emit interruptRequested();
        emit interruptTriggered(); // Сигнал для state machine
    }
}

void ControlStateMachine::triggerExit() {
    if (m_currentState == STATE_READY_CHECK || m_currentState == STATE_RESTART_EXIT) {
        emit stopChartRecording();
        // ЕСЛИ ТЕСТ ЗАПУЩЕН (не в состоянии READY_CHECK), ОСТАНАВЛИВАЕМ ЕГО
        if (m_currentState == STATE_RESTART_EXIT) {
            // Тест уже завершен или остановлен, просто выходим
            emit logMessage("Выход из завершенного теста");
        } else if (m_currentState == STATE_READY_CHECK) {
            // Если тест активен в фоне, останавливаем его
            emit stopCurrentTest();
            emit logMessage("Остановка теста и выход в меню");
        }
        resetStatusRegisters();
        writeM6Exit();
        emit exitRequested();
        emit exitTriggered();

        // СИГНАЛ ДЛЯ ОСТАНОВКИ ТЕСТА В MODE CONTROLLER
        emit stopCurrentTest();
    }
}

// Приватные методы для записи команд
void ControlStateMachine::resetStatusRegisters() {
    if (!m_client || !m_client->isConnected()) {
        qWarning() << "ControlStateMachine: Cannot reset status registers - device not connected";
        return;
    }

    // Список регистров для сброса
    QVector<QPair<QModbusDataUnit::RegisterType, quint16>> registersToReset = {
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS},
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS},
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS},
        {QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS}
    };

    for (const auto& reg : registersToReset) {
        m_client->writeRegister(reg.first, reg.second, 0);
    }

    emit logMessage("Все статусные регистры сброшены в 0");
}

void ControlStateMachine::writeM1ReadyCheck() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M1_READY_CHECK,
                          1, "M1");
}

void ControlStateMachine::writeM2Start() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M2_START,
                          1, "M2");
}

void ControlStateMachine::writeM3Stop() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M3_STOP,
                          1, "M3");
}

void ControlStateMachine::writeM4Restart() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M4_RESTART,
                          1, "M4");
}

void ControlStateMachine::writeM5Interrupt() {
    writeAndVerifyRegister(QModbusDataUnit::Coils,
                          DeltaAS332T::Addresses::M5_INTERRUPT,
                          1, "M5");
}

void ControlStateMachine::writeM6Exit() {
    if (!m_client || !m_client->isConnected()) {
        QString error = QString("[%1] Ошибка: устройство не подключено для выхода")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"));
        emit logMessage(error);
        emit registerWriteVerified("M6", 1, false);
        emit registerWriteVerified("D0", 0, false);
        return;
    }

    // Логируем операцию выхода
    logRegisterOperation("Запись регистра",
                        DeltaAS332T::Addresses::M6_EXIT,
                        1, "M6");
    logRegisterOperation("Запись регистра",
                        DeltaAS332T::Addresses::D0_MODE_REGISTER,
                        0, "D0");

    // Используем верифицированную запись
    m_client->writeAndVerifyRegister(QModbusDataUnit::Coils,
                                   DeltaAS332T::Addresses::M6_EXIT, 1, 3000);
    m_client->writeAndVerifyRegister(QModbusDataUnit::HoldingRegisters,
                                   DeltaAS332T::Addresses::D0_MODE_REGISTER, 0, 3000);

    // Через секунду сбрасываем M6=0
    QTimer::singleShot(1000, [this]() {
        if (m_client && m_client->isConnected()) {
            logRegisterOperation("Сброс регистра",
                               DeltaAS332T::Addresses::M6_EXIT,
                               0, "M6");
            m_client->writeAndVerifyRegister(QModbusDataUnit::Coils,
                                           DeltaAS332T::Addresses::M6_EXIT, 0, 3000);
        }
    });
}

void ControlStateMachine::logRegisterOperation(const QString& operation, quint16 address,
                                              quint16 value, const QString& registerName, bool success) {
    QString status = success ? "✓ УСПЕХ" : "✗ ОШИБКА";
    QString message = QString("[%1] %2: %3=%4 (адрес: 0x%5) - %6")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                         .arg(operation)
                         .arg(registerName)
                         .arg(value)
                         .arg(address, 4, 16, QChar('0'))
                         .arg(status);
    emit logMessage(message);
    qDebug() << message;
}

void ControlStateMachine::writeAndVerifyRegister(QModbusDataUnit::RegisterType type,
                                                quint16 address, quint16 value,
                                                const QString& registerName) {
    if (!m_client || !m_client->isConnected()) {
        QString error = QString("[%1] Ошибка: устройство не подключено для записи %2")
                           .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                           .arg(registerName);
        emit logMessage(error);
        emit registerWriteVerified(registerName, value, false);
        return;
    }

    // Логируем начало операции
    logRegisterOperation("Запись регистра", address, value, registerName);

    // Используем верифицированную запись
    m_client->writeAndVerifyRegister(type, address, value, 3000);

    // Через секунду сбрасываем в 0 (для импульсных команд)
    QTimer::singleShot(1000, [this, type, address, registerName]() {
        if (m_client && m_client->isConnected()) {
            logRegisterOperation("Сброс регистра", address, 0, registerName);
            m_client->writeAndVerifyRegister(type, address, 0, 3000);
        }
    });
}
