#pragma once
#include <QWidget>
#include <QVector>

class QLabel;
class QPushButton;
class QTextEdit;
class DiscreteInputMonitor;
class AnalogValueMonitor;
class DualIndicatorWidget;

class MonitorWidget : public QWidget {
    Q_OBJECT
public:
    explicit MonitorWidget(QWidget* parent = nullptr);

    void setDiscreteMonitor(DiscreteInputMonitor* monitor);
    void setAnalogMonitor(AnalogValueMonitor* monitor);

    // Новые методы для получения элементов управления (2 кнопки вместо 4)
    QPushButton* actionButton1() const { return m_actionButton1; }
    QPushButton* actionButton2() const { return m_actionButton2; }
    QTextEdit* logTextEdit() const { return m_logTextEdit; }

private:
    void setupUI();
    QWidget* createDiscreteInputsWidget();
    QWidget* createCommandOutputsWidget();
    QWidget* createAnalogValuesWidget();
    QWidget* createControlButtonsWidget(); // Теперь создает 2 кнопки
    QWidget* createMonitorLogWidget();

    QVector<QLabel*> m_discreteLabels;
    QVector<QLabel*> m_commandLabels;

    DualIndicatorWidget* m_adIndicator;
    DualIndicatorWidget* m_tkIndicator;
    DualIndicatorWidget* m_stIndicator;

    // Элементы управления (2 кнопки вместо 4)
    QPushButton* m_actionButton1; // Для: Проверка готовности, Пуск, Повторение запуска
    QPushButton* m_actionButton2; // Для: Выход, Прерывание, Стоп
    QTextEdit* m_logTextEdit;
};
