#include "WidgetFactory.h"
#include "../widgets/ConnectionWidget.h"
#include "../widgets/MonitorWidget.h"
#include "../widgets/ChartWidget.h"
#include <QVBoxLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QFormLayout>
#include <QGridLayout>

WidgetFactory::WidgetFactory(QObject* parent)
    : QObject(parent)
    , m_modeButton1(nullptr)
    , m_modeButton2(nullptr)
    , m_modeButton3(nullptr)
    , m_modeButton4(nullptr)
    , m_modeButton5(nullptr)
    , m_stopButton(nullptr)
    , m_paramCombo(nullptr)
    , m_addressEdit(nullptr)
    , m_addParamButton(nullptr)
    , m_readyCheckButton(nullptr)
    , m_controlStartButton(nullptr)
    , m_restartButton(nullptr)
    , m_exitButton(nullptr)
{}

ConnectionWidget* WidgetFactory::createConnectionWidget() {
    return new ConnectionWidget();
}

MonitorWidget* WidgetFactory::createMonitorWidget() {
    return new MonitorWidget();
}

ChartWidget* WidgetFactory::createChartWidget(IDataRepository* repository) {
    ChartWidget* widget = new ChartWidget();
    widget->setDataRepository(repository);
    return widget;
}

QWidget* WidgetFactory::createModeControlWidget() {
    QGroupBox* group = new QGroupBox("Управление режимами тестирования");
    QGridLayout* layout = new QGridLayout(group);

    // Создаем 5 кнопок режимов и 1 кнопку остановки
    m_modeButton1 = new QPushButton("Расконсервация/\nКонсервация");
    m_modeButton2 = new QPushButton("Холодная прокрутка\nтурбостартера");
    m_modeButton3 = new QPushButton("Регулировка мощности,\nзамер параметров");
    m_modeButton4 = new QPushButton("Холодная прокрутка\nосновного двигателя");
    m_modeButton5 = new QPushButton("Имитация запуска\nосновного двигателя");
    m_stopButton = new QPushButton("СТОП ТЕСТА");

    // Настраиваем размеры кнопок
    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
    for (QPushButton* button : modeButtons) {
        button->setMinimumSize(180, 80);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    m_stopButton->setMinimumSize(180, 80);
    m_stopButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Настраиваем стили для неактивного состояния (по умолчанию)
    QString disabledStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #7f8c8d;"
        "border-radius: 8px;"
        "background-color: #bdc3c7;"
        "color: #7f8c8d;"
        "}";

    QString activeModeStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 8px;"
        "background-color: #3498db;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}";

    QString activeStopStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #c0392b;"
        "border-radius: 8px;"
        "background-color: #e74c3c;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #c0392b;"
        "}"
        "QPushButton:pressed {"
        "background-color: #a93226;"
        "}";

    QString activeGreenStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 11px;"
        "padding: 8px;"
        "border: 2px solid #27ae60;"
        "border-radius: 8px;"
        "background-color: #2ecc71;"
        "color: white;"
        "}";

    // Применяем неактивный стиль по умолчанию
    for (QPushButton* button : modeButtons) {
        button->setStyleSheet(disabledStyle);
        button->setEnabled(false);
    }
    m_stopButton->setStyleSheet(disabledStyle);
    m_stopButton->setEnabled(false);

    // Сохраняем стили как свойства для последующего использования
    m_modeButton1->setProperty("activeStyle", activeModeStyle);
    m_modeButton1->setProperty("disabledStyle", disabledStyle);
    m_modeButton1->setProperty("activeGreenStyle", activeGreenStyle);

    // Располагаем кнопки в сетке 2x3
    layout->addWidget(m_modeButton1, 0, 0);
    layout->addWidget(m_modeButton2, 0, 1);
    layout->addWidget(m_modeButton3, 0, 2);
    layout->addWidget(m_modeButton4, 1, 0);
    layout->addWidget(m_modeButton5, 1, 1);
    layout->addWidget(m_stopButton, 1, 2);

    // Устанавливаем растягивание столбцов
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(0, 1);
    layout->setRowStretch(1, 1);

    return group;
}

QWidget* WidgetFactory::createStateControlWidget() {
    QGroupBox* group = new QGroupBox("Управление запуском");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем только две кнопки для всех состояний
    m_actionButton1 = new QPushButton("Проверка готовности");
    m_actionButton2 = new QPushButton("Выход");

    // Настраиваем размеры кнопок
    m_actionButton1->setMinimumHeight(40);
    m_actionButton2->setMinimumHeight(40);

    // Базовый стиль
    QString buttonStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 12px;"
        "padding: 8px 12px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 5px;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "background-color: #bdc3c7;"
        "border-color: #95a5a6;"
        "color: #7f8c8d;"
        "}";

    m_actionButton1->setStyleSheet(buttonStyle + "background-color: #3498db;");
    m_actionButton2->setStyleSheet(buttonStyle + "background-color: #e74c3c;");

    layout->addWidget(m_actionButton1);
    layout->addWidget(m_actionButton2);

    return group;
}


QWidget* WidgetFactory::createParameterControlWidget() {
    return nullptr;
}

QTextEdit* WidgetFactory::createLogWidget() {
    QTextEdit* logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(200);
    return logEdit;
}

QWidget* WidgetFactory::createControlButtonsWidget() {
    QGroupBox* group = new QGroupBox("Управление запуском");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем кнопки управления (теперь 4 вместо 5)
    m_readyCheckButton = new QPushButton("Проверка готовности");
    m_controlStartButton = new QPushButton("ПУСК"); // Будет меняться на СТОП
    m_restartButton = new QPushButton("Повторение запуска"); // Будет меняться на Прерывание
    m_exitButton = new QPushButton("ВЫХОД");

    // Настраиваем размеры кнопок
    m_readyCheckButton->setMinimumHeight(40);
    m_controlStartButton->setMinimumHeight(40);
    m_restartButton->setMinimumHeight(40);
    m_exitButton->setMinimumHeight(40);

    // Базовый стиль
    QString buttonStyle =
        "QPushButton {"
        "font-weight: bold;"
        "font-size: 12px;"
        "padding: 8px 12px;"
        "border: 2px solid #2c3e50;"
        "border-radius: 5px;"
        "background-color: #3498db;"
        "color: white;"
        "}"
        "QPushButton:hover {"
        "background-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "background-color: #21618c;"
        "}"
        "QPushButton:disabled {"
        "background-color: #bdc3c7;"
        "border-color: #95a5a6;"
        "color: #7f8c8d;"
        "}";

    m_readyCheckButton->setStyleSheet(buttonStyle);
    m_controlStartButton->setStyleSheet(buttonStyle);
    m_restartButton->setStyleSheet(buttonStyle);
    m_exitButton->setStyleSheet(buttonStyle);

    // Начальное состояние кнопок
    m_readyCheckButton->setEnabled(true);
    m_controlStartButton->setEnabled(true);
    m_restartButton->setEnabled(true);
    m_exitButton->setEnabled(true);

    // Добавляем кнопки в layout
    layout->addWidget(m_readyCheckButton);
    layout->addWidget(m_controlStartButton);
    layout->addWidget(m_restartButton);
    layout->addWidget(m_exitButton);

    return group;
}

QTextEdit* WidgetFactory::createMonitorLogWidget() {
    QTextEdit* logEdit = new QTextEdit();
    logEdit->setReadOnly(true);
    logEdit->setMaximumHeight(150);
    logEdit->setPlaceholderText("Журнал событий мониторинга...");
    return logEdit;
}
