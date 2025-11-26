#pragma once
#include <QObject>
#include "ControlStateMachine.h"  // Включаем полное определение

class QPushButton;

class ControlUIController : public QObject {
    Q_OBJECT
public:
    explicit ControlUIController(ControlStateMachine* stateMachine, QObject* parent = nullptr);

    void setControlButtons(QPushButton* actionButton1, QPushButton* actionButton2);
    inline bool isInitialized() const {return m_initialized;}

public slots:
    void onStateChanged(ControlStateMachine::State newState);
    void onActionButton1Clicked();
    void onActionButton2Clicked();
    void forceUpdateUI();

private:
    void updateUIForState(ControlStateMachine::State state);

    ControlStateMachine* m_stateMachine;
    QPushButton* m_actionButton1; // Кнопка для: Проверка готовности, Пуск, Повторение запуска
    QPushButton* m_actionButton2; // Кнопка для: Выход, Прерывание, Стоп
    bool m_initialized; // Защита от повторной инициализации
};
