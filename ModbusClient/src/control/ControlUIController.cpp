#include "ControlUIController.h"
#include "ControlStateMachine.h"
#include <QPushButton>
#include <QDebug>
#include <QTimer>

ControlUIController::ControlUIController(ControlStateMachine* stateMachine, QObject* parent)
    : QObject(parent)
    , m_stateMachine(stateMachine)
    , m_actionButton1(nullptr)
    , m_actionButton2(nullptr)
    , m_initialized(false)
{
    qDebug() << "ControlUIController created";

    if (m_stateMachine) {
        connect(m_stateMachine, &ControlStateMachine::stateChanged,
                this, &ControlUIController::onStateChanged, Qt::QueuedConnection); // Используем QueuedConnection
    }
}

void ControlUIController::setControlButtons(QPushButton* actionButton1, QPushButton* actionButton2) {
    qDebug() << "ControlUIController::setControlButtons called";

    // Защита от повторного вызова
    if (m_initialized) {
        qWarning() << "ControlUIController already initialized, ignoring duplicate call";
        return;
    }

    if (!actionButton1 || !actionButton2) {
        qCritical() << "ControlUIController: One or both buttons are null!";
        return;
    }

    // Проверяем, что кнопки еще "живы"
    if (actionButton1->isWidgetType() && actionButton2->isWidgetType()) {
        qDebug() << "Buttons are valid widgets";
    } else {
        qCritical() << "Buttons are not valid widgets!";
        return;
    }

    // Сохраняем новые кнопки
    m_actionButton1 = actionButton1;
    m_actionButton2 = actionButton2;

    qDebug() << "Buttons set successfully:"
             << "Button1:" << (void*)m_actionButton1
             << "Button2:" << (void*)m_actionButton2;

    // Безопасно отключаем ВСЕ существующие соединения от этих кнопок
    if (m_actionButton1) {
        try {
            m_actionButton1->disconnect();
            qDebug() << "Disconnected all signals from button1";
        } catch (...) {
            qCritical() << "Failed to disconnect button1";
            return;
        }
    }

    if (m_actionButton2) {
        try {
            m_actionButton2->disconnect();
            qDebug() << "Disconnected all signals from button2";
        } catch (...) {
            qCritical() << "Failed to disconnect button2";
            return;
        }
    }

    // Подключаем новые соединения с проверкой
    bool connect1 = false, connect2 = false;

    if (m_actionButton1) {
        connect1 = connect(m_actionButton1, &QPushButton::clicked,
                          this, &ControlUIController::onActionButton1Clicked,
                          Qt::QueuedConnection);
        qDebug() << "Button1 connection:" << (connect1 ? "success" : "failed");
    }

    if (m_actionButton2) {
        connect2 = connect(m_actionButton2, &QPushButton::clicked,
                          this, &ControlUIController::onActionButton2Clicked,
                          Qt::QueuedConnection);
        qDebug() << "Button2 connection:" << (connect2 ? "success" : "failed");
    }

    if (!connect1 || !connect2) {
        qCritical() << "Failed to connect one or both buttons";
        return;
    }

    // Помечаем как инициализированный
    m_initialized = true;

    // Откладываем обновление UI до следующего цикла событий
    QTimer::singleShot(0, this, [this]() {
        if (m_stateMachine && m_actionButton1 && m_actionButton2) {
            qDebug() << "Performing initial UI update";
            updateUIForState(m_stateMachine->currentState());
        } else {
            qCritical() << "Cannot perform initial UI update";
        }
    });

    qDebug() << "ControlUIController initialization completed successfully";
}

void ControlUIController::onStateChanged(ControlStateMachine::State newState) {
    qDebug() << "ControlUIController: State changed to" << newState;

    if (!m_actionButton1 || !m_actionButton2) {
        qWarning() << "ControlUIController: Buttons not available for UI update";
        return;
    }

    updateUIForState(newState);
    qDebug() << "UI update completed for state:" << newState;
}

void ControlUIController::updateUIForState(ControlStateMachine::State state) {
    qDebug() << "ControlUIController: Updating UI for state" << state;

    if (!m_actionButton1 || !m_actionButton2) {
        qCritical() << "ControlUIController: Cannot update UI - buttons are null";
        return;
    }

    // Временно блокируем сигналы чтобы избежать рекурсии
    m_actionButton1->blockSignals(true);
    m_actionButton2->blockSignals(true);

    try {
        // Сбрасываем стили
        m_actionButton1->setStyleSheet("");
        m_actionButton2->setStyleSheet("");

        switch (state) {
        case ControlStateMachine::STATE_READY_CHECK:
            qDebug() << "Setting state: READY_CHECK";
            m_actionButton1->setText("Проверка готовности");
            m_actionButton1->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #3498db; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #2980b9; }"
                "QPushButton:pressed { background-color: #21618c; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton1->setEnabled(true);
            m_actionButton1->setVisible(true);

            m_actionButton2->setText("Выход");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #e74c3c; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #c0392b; }"
                "QPushButton:pressed { background-color: #a93226; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;

        case ControlStateMachine::STATE_START_INTERRUPT:
            qDebug() << "Setting state: START_INTERRUPT";
            m_actionButton1->setText("ПУСК");
            m_actionButton1->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #27ae60; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #229954; }"
                "QPushButton:pressed { background-color: #1e8449; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton1->setEnabled(true);
            m_actionButton1->setVisible(true);

            m_actionButton2->setText("Прерывание\nзапуска");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #f39c12; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #d68910; }"
                "QPushButton:pressed { background-color: #b9770e; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;

        case ControlStateMachine::STATE_STOP:
            qDebug() << "Setting state: STOP";
            m_actionButton1->setText("");
            m_actionButton1->setStyleSheet("");
            m_actionButton1->setEnabled(false);
            m_actionButton1->setVisible(false);

            m_actionButton2->setText("СТОП");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #c0392b; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #a93226; }"
                "QPushButton:pressed { background-color: #922b21; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;

        case ControlStateMachine::STATE_RESTART_EXIT:
            qDebug() << "Setting state: RESTART_EXIT";
            m_actionButton1->setText("Повторение\nзапуска");
            m_actionButton1->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #9b59b6; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #8e44ad; }"
                "QPushButton:pressed { background-color: #7d3c98; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton1->setEnabled(true);
            m_actionButton1->setVisible(true);

            m_actionButton2->setText("Выход");
            m_actionButton2->setStyleSheet(
                "QPushButton {"
                "font-weight: bold; font-size: 12px; padding: 8px 12px;"
                "border: 2px solid #2c3e50; border-radius: 5px;"
                "background-color: #e74c3c; color: white;"
                "min-height: 40px; min-width: 150px;"
                "}"
                "QPushButton:hover { background-color: #c0392b; }"
                "QPushButton:pressed { background-color: #a93226; }"
                "QPushButton:disabled { background-color: #bdc3c7; color: #7f8c8d; }"
            );
            m_actionButton2->setEnabled(true);
            m_actionButton2->setVisible(true);
            break;
        }

        // Принудительно обновляем отображение
        m_actionButton1->update();
        m_actionButton2->update();

        qDebug() << "UI update completed successfully";

    } catch (const std::exception& e) {
        qCritical() << "Exception in updateUIForState:" << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in updateUIForState";
    }

    // Разблокируем сигналы
    m_actionButton1->blockSignals(false);
    m_actionButton2->blockSignals(false);
}

void ControlUIController::onActionButton1Clicked() {
    qDebug() << "ControlUIController: Action button 1 clicked";

    if (!m_stateMachine) {
        qWarning() << "State machine not available";
        return;
    }

    ControlStateMachine::State currentState = m_stateMachine->currentState();
    qDebug() << "Current state:" << currentState;

    switch (currentState) {
    case ControlStateMachine::STATE_READY_CHECK:
        qDebug() << "Triggering ready check";
        m_stateMachine->triggerReadyCheck();
        break;
    case ControlStateMachine::STATE_START_INTERRUPT:
        qDebug() << "Triggering start";
        m_stateMachine->triggerStart();
        break;
    case ControlStateMachine::STATE_RESTART_EXIT:
        qDebug() << "Triggering restart";
        m_stateMachine->triggerRestart();
        break;
    case ControlStateMachine::STATE_STOP:
        qDebug() << "Button 1 inactive in STOP state";
        break;
    }
}

void ControlUIController::onActionButton2Clicked() {
    qDebug() << "ControlUIController: Action button 2 clicked";

    if (!m_stateMachine) {
        qWarning() << "State machine not available";
        return;
    }

    ControlStateMachine::State currentState = m_stateMachine->currentState();
    qDebug() << "Current state:" << currentState;

    switch (currentState) {
    case ControlStateMachine::STATE_READY_CHECK:
    case ControlStateMachine::STATE_RESTART_EXIT:
        qDebug() << "Triggering exit";
        m_stateMachine->triggerExit();
        break;
    case ControlStateMachine::STATE_START_INTERRUPT:
        qDebug() << "Triggering interrupt";
        m_stateMachine->triggerInterrupt();
        break;
    case ControlStateMachine::STATE_STOP:
        qDebug() << "Triggering stop";
        m_stateMachine->triggerStop();
        break;
    }
}

void ControlUIController::forceUpdateUI() {
    qDebug() << "ControlUIController: Force updating UI";
    if (m_stateMachine) {
        updateUIForState(m_stateMachine->currentState());
    }
}
