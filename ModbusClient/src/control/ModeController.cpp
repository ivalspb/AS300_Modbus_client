#include "ModeController.h"
#include "core/interfaces/IModbusClient.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QPushButton>
#include <QDateTime>
#include <QModbusDataUnit>
#include <QTimer>
#include <QDebug>

ModeController::ModeController(IModbusClient* client, QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_modeButton1(nullptr)
    , m_modeButton2(nullptr)
    , m_modeButton3(nullptr)
    , m_modeButton4(nullptr)
    , m_modeButton5(nullptr)
    , m_stopButton(nullptr)
    , m_connected(false)
    , m_testRunning(false)
{
    initializeModes();

    if (m_client) {
        connect(m_client, &IModbusClient::registerWriteVerified,
                this, &ModeController::onRegisterWriteVerified);
    }
}

void ModeController::initializeModes() {
    m_modeMap.clear();
    m_modeMap["Расконсервация/Консервация"] = "DECONSERVATION";
    m_modeMap["Холодная прокрутка турбостартера"] = "COLD_CRANK_TURBO";
    m_modeMap["Регулировка мощности, замер параметров"] = "POWER_CONTROL";
    m_modeMap["Холодная прокрутка основного двигателя"] = "COLD_CRANK_MAIN";
    m_modeMap["Имитация запуска основного двигателя"] = "SIMULATION";
}

void ModeController::setModeButtons(QPushButton* mode1, QPushButton* mode2, QPushButton* mode3,
                                   QPushButton* mode4, QPushButton* mode5, QPushButton* stopButton) {
    m_modeButton1 = mode1;
    m_modeButton2 = mode2;
    m_modeButton3 = mode3;
    m_modeButton4 = mode4;
    m_modeButton5 = mode5;
    m_stopButton = stopButton;

    if (m_modeButton1) {
        connect(m_modeButton1, &QPushButton::clicked, this, &ModeController::onMode1Clicked);
    }
    if (m_modeButton2) {
        connect(m_modeButton2, &QPushButton::clicked, this, &ModeController::onMode2Clicked);
    }
    if (m_modeButton3) {
        connect(m_modeButton3, &QPushButton::clicked, this, &ModeController::onMode3Clicked);
    }
    if (m_modeButton4) {
        connect(m_modeButton4, &QPushButton::clicked, this, &ModeController::onMode4Clicked);
    }
    if (m_modeButton5) {
        connect(m_modeButton5, &QPushButton::clicked, this, &ModeController::onMode5Clicked);
    }
    if (m_stopButton) {
        connect(m_stopButton, &QPushButton::clicked, this, &ModeController::onStopClicked);
    }

    updateButtonStates();
}

void ModeController::setConnectionState(bool connected) {
    m_connected = connected;
    updateButtonStates();
}

void ModeController::updateButtonStates() {
    QString disabledStyle = "QPushButton { font-weight: bold; font-size: 11px; padding: 8px; border: 2px solid #7f8c8d; border-radius: 8px; background-color: #bdc3c7; color: #7f8c8d; }";
    QString activeModeStyle = "QPushButton { font-weight: bold; font-size: 11px; padding: 8px; border: 2px solid #2c3e50; border-radius: 8px; background-color: #3498db; color: white; } QPushButton:hover { background-color: #2980b9; } QPushButton:pressed { background-color: #21618c; }";
    QString activeStopStyle = "QPushButton { font-weight: bold; font-size: 11px; padding: 8px; border: 2px solid #c0392b; border-radius: 8px; background-color: #e74c3c; color: white; } QPushButton:hover { background-color: #c0392b; } QPushButton:pressed { background-color: #a93226; }";
    QString activeGreenStyle = "QPushButton { font-weight: bold; font-size: 11px; padding: 8px; border: 2px solid #27ae60; border-radius: 8px; background-color: #2ecc71; color: white; }";

    if (!m_connected) {
        // Все кнопки серые и неактивные при отключении
        QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
        for (QPushButton* button : modeButtons) {
            if (button) {
                button->setStyleSheet(disabledStyle);
                button->setEnabled(false);
            }
        }
        if (m_stopButton) {
            m_stopButton->setStyleSheet(disabledStyle);
            m_stopButton->setEnabled(false);
        }
    }
    else if (m_testRunning) {
        // Тест запущен: все кнопки режимов серые (кроме запущеной), стоп красная
        QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
        for (QPushButton* button : modeButtons) {
            if (button) {
//                button->setStyleSheet(disabledStyle);
                button->setEnabled(false);
            }
        }
        if (m_stopButton) {
            m_stopButton->setStyleSheet(activeStopStyle);
            m_stopButton->setEnabled(true);
        }
    }
    else {
        // Подключено, тест не запущен: кнопки режимов активные, стоп серая
        QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
        for (QPushButton* button : modeButtons) {
            if (button) {
                button->setStyleSheet(activeModeStyle);
                button->setEnabled(true);
            }
        }
        if (m_stopButton) {
            m_stopButton->setStyleSheet(disabledStyle);
            m_stopButton->setEnabled(false);
        }
    }
}

void ModeController::setActiveModeButton(QPushButton* activeButton) {
    QString activeGreenStyle = "QPushButton { font-weight: bold; font-size: 11px; padding: 8px; border: 2px solid #27ae60; border-radius: 8px; background-color: #2ecc71; color: white; }";
    QString disabledStyle = "QPushButton { font-weight: bold; font-size: 11px; padding: 8px; border: 2px solid #7f8c8d; border-radius: 8px; background-color: #bdc3c7; color: #7f8c8d; }";

    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
    for (QPushButton* button : modeButtons) {
        if (button && button != activeButton) {
            button->setStyleSheet(disabledStyle);
            button->setEnabled(false);
        } else if (button == activeButton) {
            button->setStyleSheet(activeGreenStyle);
            button->setEnabled(false); // Активная кнопка тоже неактивна до подтверждения
        }
    }
}

// Обработчики нажатий кнопок режимов
void ModeController::onMode1Clicked() { startTest("Расконсервация/Консервация", m_modeButton1); }
void ModeController::onMode2Clicked() { startTest("Холодная прокрутка турбостартера", m_modeButton2); }
void ModeController::onMode3Clicked() { startTest("Регулировка мощности, замер параметров", m_modeButton3); }
void ModeController::onMode4Clicked() { startTest("Холодная прокрутка основного двигателя", m_modeButton4); }
void ModeController::onMode5Clicked() { startTest("Имитация запуска основного двигателя", m_modeButton5); }

void ModeController::startTest(const QString& mode, QPushButton* button) {
    if (!m_connected || m_testRunning) return;

    m_currentMode = mode;
    m_testRunning = true;

    // Визуально отмечаем активную кнопку
    setActiveModeButton(button);

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

        emit logMessage(QString("[%1] Запуск теста: %2 (D0=%3)")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                            .arg(m_currentMode)
                            .arg(d0Value));
    }

    emit modeChanged(m_currentMode);
}

void ModeController::onStopClicked() {
    if (!m_testRunning) return;

    // Send stop command via Modbus
    if (m_client && m_client->isConnected()) {
        m_client->writeRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);
    }

    m_testRunning = false;
    m_currentMode.clear();

    emit testStopped();
    emit logMessage(QString("[%1] Тест остановлен")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

    // Возвращаем кнопки в исходное состояние
    updateButtonStates();
}

void ModeController::onRegisterWriteVerified(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, bool success) {
    if (address == DeltaAS332T::Addresses::D0_MODE_REGISTER) { // D0 register
        if (success) {
            emit logMessage(QString("[%1] ✓ Режим установлен: D0=%2")
                                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                .arg(value));

            // После подтверждения записи D0 запускаем тест
            emit testStarted(m_currentMode);

            // Обновляем состояние кнопок (стоп становится активной)
            updateButtonStates();

            qDebug() << "ModeController: Test started successfully for mode:" << m_currentMode;
        } else {
            emit logMessage(QString("[%1] ✗ Ошибка установки режима: D0 не установлен (прочитано: %2)")
                                .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
                                .arg(value));

            // При ошибке возвращаем кнопки в исходное состояние
            m_testRunning = false;
            m_currentMode.clear();
            updateButtonStates();

            qDebug() << "ModeController: Failed to start test for mode:" << m_currentMode;
        }
    }
}

void ModeController::stopTest() {
    if (m_testRunning) {
        m_testRunning = false;
        m_currentMode.clear();

        // Сбрасываем D0 в 0
        if (m_client && m_client->isConnected()) {
            m_client->writeRegister(QModbusDataUnit::HoldingRegisters,
                                  DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);
        }

        // Сбрасываем UI кнопок
        updateButtonStates();

        emit testStopped();
        emit logMessage(QString("[%1] Тест принудительно остановлен")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

        qDebug() << "ModeController: Test stopped by exit command";
    }
}
