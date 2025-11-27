#include "ModeController.h"
#include "core/interfaces/IModbusClient.h"
#include "core/mapping/DeltaAddressMap.h"
#include "data/interfaces/IDataRepository.h"
#include <QPushButton>
#include <QDateTime>
#include <QModbusDataUnit>
#include <QTimer>
#include <QDebug>

ModeController::ModeController(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_dataRepository(nullptr)
    , m_connected(false)
    , m_testRunning(false)
{
    if (m_client) {
        connect(m_client, &IModbusClient::registerWriteVerified,
                this, &ModeController::onRegisterWriteVerified);
    }

    qDebug() << "ModeController: Created";
}

void ModeController::setConnectionState(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        qDebug() << "ModeController: Connection state changed to:" << connected;
        emit logMessage(connected ? "Устройство подключено" : "Устройство отключено");
    }
}

// Обработчики нажатий кнопок режимов
void ModeController::onMode1Clicked() {
    qDebug() << "ModeController: Mode 1 clicked";
    startTest("Расконсервация/Консервация");
}

void ModeController::onMode2Clicked() {
    qDebug() << "ModeController: Mode 2 clicked";
    startTest("Холодная прокрутка турбостартера");
}

void ModeController::onMode3Clicked() {
    qDebug() << "ModeController: Mode 3 clicked";
    startTest("Регулировка мощности, замер параметров");
}

void ModeController::onMode4Clicked() {
    qDebug() << "ModeController: Mode 4 clicked";
    startTest("Холодная прокрутка основного двигателя");
}

void ModeController::onMode5Clicked() {
    qDebug() << "ModeController: Mode 5 clicked";
    startTest("Имитация запуска основного двигателя");
}

void ModeController::startTest(const QString& mode) {
    if (!m_connected || m_testRunning) {
        qWarning() << "ModeController: Cannot start test - connected:" << m_connected
                   << "testRunning:" << m_testRunning;
        return;
    }

    m_currentMode = mode;
    m_testRunning = true;

    emit logMessage(QString("Запуск теста: %1").arg(m_currentMode));
    qDebug() << "ModeController: Starting test:" << m_currentMode;

    // Создаём сессию в репозитории
    if (m_dataRepository) {

        // Подключаемся к сигналу создания сессии
        connect(m_dataRepository, &IDataRepository::sessionCreated,
                this, &ModeController::onSessionCreated, Qt::UniqueConnection);

        m_dataRepository->setCurrentTestSession(m_currentMode);
        qDebug() << "ModeController: Test session created in repository";
    } else {
        qWarning() << "ModeController: DataRepository not set!";
    }
    writeModeRegister();
}

void ModeController::onSessionCreated(int sessionId) {
    qDebug() << "ModeController: Session created with ID:" << sessionId;

    // Теперь когда сессия создана, можно писать в D0
    writeModeRegister();
}

void ModeController::writeModeRegister() {
    // Запись в D0 согласно алгоритму
    if (m_client && m_client->isConnected()) {
        quint16 d0Value = 0;
        if (m_currentMode == "Расконсервация/Консервация") d0Value = 1;
        else if (m_currentMode == "Холодная прокрутка турбостартера") d0Value = 2;
        else if (m_currentMode == "Регулировка мощности, замер параметров") d0Value = 3;
        else if (m_currentMode == "Холодная прокрутка основного двигателя") d0Value = 4;
        else if (m_currentMode == "Имитация запуска основного двигателя") d0Value = 5;

        m_client->writeAndVerifyRegister(QModbusDataUnit::HoldingRegisters,
                                         DeltaAS332T::Addresses::D0_MODE_REGISTER, d0Value, 5000);

        emit logMessage(QString("[%1] Установка режима: %2 (D0=%3)")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                            .arg(m_currentMode)
                            .arg(d0Value));

        qDebug() << "ModeController: Mode register D0 set to" << d0Value;
    }

    emit modeChanged(m_currentMode);
}

void ModeController::onStopClicked() {
    qDebug() << "ModeController: Stop clicked";

    if (!m_testRunning) {
        qWarning() << "ModeController: No test running to stop";
        return;
    }

    // Send stop command via Modbus
    if (m_client && m_client->isConnected()) {
        m_client->writeRegister(QModbusDataUnit::HoldingRegisters,
                                DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);
    }

    // Сохраняем сессию перед остановкой
    if (m_dataRepository) {
        m_dataRepository->saveCurrentSessionToDatabase();
        qDebug() << "ModeController: Test session saved on stop";
    }

    m_testRunning = false;
    m_currentMode.clear();

    emit testStopped();
    emit logMessage(QString("[%1] Тест остановлен")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

    qDebug() << "ModeController: Test stopped by user";
}

void ModeController::onRegisterWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success) {
    if (address == DeltaAS332T::Addresses::D0_MODE_REGISTER) {
        if (success) {
            emit logMessage(QString("[%1] ✓ Режим установлен: D0=%2")
                                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                .arg(value));

            // После подтверждения записи D0 запускаем тест
            if (value > 0 && m_testRunning) {
                emit testStarted(m_currentMode);
                qDebug() << "ModeController: Test started successfully for mode:" << m_currentMode;
            }
        } else {
            emit logMessage(QString("[%1] ✗ Ошибка установки режима: D0 не установлен (прочитано: %2)")
                                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                .arg(value));

            // При ошибке возвращаем состояние
            m_testRunning = false;
            m_currentMode.clear();
            emit testStopped();

            qDebug() << "ModeController: Failed to start test for mode:" << m_currentMode;
        }
    }
}

void ModeController::stopTest() {
    if (m_testRunning) {
        qDebug() << "ModeController: Stopping test (forced)";

        // Сохраняем сессию перед остановкой
        if (m_dataRepository) {
            m_dataRepository->saveCurrentSessionToDatabase();
            qDebug() << "ModeController: Test session saved on forced stop";
        }

        m_testRunning = false;
        m_currentMode.clear();

        // Сбрасываем D0 в 0
        if (m_client && m_client->isConnected()) {
            m_client->writeRegister(QModbusDataUnit::HoldingRegisters,
                                    DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);
        }

        emit testStopped();
        emit logMessage(QString("[%1] Тест принудительно остановлен")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

        qDebug() << "ModeController: Test stopped by exit command";
    }
}
