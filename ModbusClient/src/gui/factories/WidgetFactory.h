#pragma once
#include <QObject>

class IModbusClient;
class IDataRepository;
class ConnectionWidget;
class MonitorWidget;
class ChartWidget;
class QComboBox;
class QPushButton;
class QLineEdit;
class QTextEdit;

class WidgetFactory : public QObject {
    Q_OBJECT
public:
    explicit WidgetFactory(QObject* parent = nullptr);

    ConnectionWidget* createConnectionWidget();
    MonitorWidget* createMonitorWidget();
    ChartWidget* createChartWidget(IDataRepository* repository);

    QWidget* createStateControlWidget();

    QWidget* createModeControlWidget(); // Теперь возвращает 6 кнопок
    QWidget* createParameterControlWidget();
    QTextEdit* createLogWidget();

    // Новые методы для создания элементов управления
    QWidget* createControlButtonsWidget();
    QTextEdit* createMonitorLogWidget();

    // Getters for controls - теперь для кнопок режимов
    QPushButton* modeButton1() const { return m_modeButton1; }
    QPushButton* modeButton2() const { return m_modeButton2; }
    QPushButton* modeButton3() const { return m_modeButton3; }
    QPushButton* modeButton4() const { return m_modeButton4; }
    QPushButton* modeButton5() const { return m_modeButton5; }
    QPushButton* stopButton() const { return m_stopButton; }

    // Добавляем геттеры для новых кнопок
    QPushButton* actionButton1() const { return m_actionButton1; }
    QPushButton* actionButton2() const { return m_actionButton2; }


    QComboBox* parameterComboBox() const { return m_paramCombo; }
    QLineEdit* addressEdit() const { return m_addressEdit; }
    QPushButton* addParameterButton() const { return m_addParamButton; }

    // Getters for control buttons
    QPushButton* readyCheckButton() const { return m_readyCheckButton; }
    QPushButton* controlStartButton() const { return m_controlStartButton; }
    QPushButton* restartButton() const { return m_restartButton; }
    QPushButton* exitButton() const { return m_exitButton; }

private:
    // Кнопки режимов вместо ComboBox
    QPushButton* m_modeButton1;
    QPushButton* m_modeButton2;
    QPushButton* m_modeButton3;
    QPushButton* m_modeButton4;
    QPushButton* m_modeButton5;
    QPushButton* m_stopButton;

    QComboBox* m_paramCombo;
    QLineEdit* m_addressEdit;
    QPushButton* m_addParamButton;

    // Кнопки управления для мониторинга
    QPushButton* m_readyCheckButton;
    QPushButton* m_controlStartButton;
    QPushButton* m_restartButton;
    QPushButton* m_exitButton;

    QPushButton* m_actionButton1;
    QPushButton* m_actionButton2;
};
