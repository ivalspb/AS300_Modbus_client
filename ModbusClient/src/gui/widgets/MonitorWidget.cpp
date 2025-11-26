#include "MonitorWidget.h"
#include "monitoring/DiscreteInputMonitor.h"
#include "monitoring/AnalogValueMonitor.h"
#include "DualIndicatorWidget.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>
#include <QTextEdit>

MonitorWidget::MonitorWidget(QWidget* parent)
    : QWidget(parent)
    , m_adIndicator(nullptr)
    , m_tkIndicator(nullptr)
    , m_stIndicator(nullptr)
    , m_actionButton1(nullptr)
    , m_actionButton2(nullptr)
    , m_logTextEdit(nullptr)
{
    setupUI();
}

void MonitorWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Верхняя часть - дискретные сигналы в двух колонках
    QHBoxLayout* discreteLayout = new QHBoxLayout();
    discreteLayout->addWidget(createDiscreteInputsWidget());
    discreteLayout->addWidget(createCommandOutputsWidget());
    layout->addLayout(discreteLayout);

    // Аналоговые значения со спидометрами
    layout->addWidget(createAnalogValuesWidget());

    // Кнопки управления (теперь 2 кнопки)
    layout->addWidget(createControlButtonsWidget());

    // Журнал событий мониторинга
    layout->addWidget(createMonitorLogWidget());

    layout->addStretch();
}

QWidget* MonitorWidget::createDiscreteInputsWidget() {
    QGroupBox* group = new QGroupBox("Дискретные входы (S1-S12)");
    QGridLayout* layout = new QGridLayout(group);
    layout->setVerticalSpacing(5);
    layout->setHorizontalSpacing(10);

    QStringList labels = {
        "S1: Откл БУТС", "S2: ОРТС", "S3: ЭМЗС", "S4: АЗТС",
        "S5: ЭСТС", "S6: Откл ЭСТС (СОЭС)", "S7: ИП АЗТС Вкл.", "S8: ИП ЭСТС Вкл.",
        "S9: 1й канал АЗТС Вкл.", "S10: 2й канал АЗТС Вкл.", "S11: ЭСТС Вкл.", "S12: ПЧ готов"
    };

    // Распределяем по 6 элементов в каждом столбце
    for (int i = 0; i < labels.size(); ++i) {
        int row = i % 6;
        int column = i / 6;

        QLabel* nameLabel = new QLabel(labels[i]);
        nameLabel->setMinimumWidth(180);
        nameLabel->setStyleSheet("background-color: grey; color: white; padding: 6px; border-radius: 3px;");
        nameLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(nameLabel, row, column);
        m_discreteLabels.append(nameLabel);
    }

    return group;
}

QWidget* MonitorWidget::createCommandOutputsWidget() {
    QGroupBox* group = new QGroupBox("Командные выходы (K1-K6)");
    QGridLayout* layout = new QGridLayout(group);

    QStringList labels = {
        "K1: Пуск ТС", "K2: Стоп ТС", "K3: СТОП-кран",
        "K4: Режим Консервации", "K5: Режим Холодной прокрутки", "K6: Активация Выходов ПЧ"
    };

    for (int i = 0; i < labels.size(); ++i) {
        QLabel* nameLabel = new QLabel(labels[i]);
        nameLabel->setMinimumWidth(200);
        nameLabel->setStyleSheet("background-color: grey; color: white; padding: 8px; border-radius: 4px;");
        nameLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(nameLabel, i, 0);
        m_commandLabels.append(nameLabel);
    }

    return group;
}

QWidget* MonitorWidget::createAnalogValuesWidget() {
    QGroupBox* group = new QGroupBox("Аналоговые значения");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем композитные индикаторы
    m_adIndicator = new DualIndicatorWidget("АД %", "об/мин", 0, 110, 0, 4996);
    m_tkIndicator = new DualIndicatorWidget("ТК %", "об/мин", 0, 110, 0, 71500);
    m_stIndicator = new DualIndicatorWidget("СТ %", "об/мин", 0, 110, 0, 64750);

    // Настройка цветов
    m_adIndicator->setColor(Qt::blue);
    m_tkIndicator->setColor(Qt::red);
    m_stIndicator->setColor(Qt::green);

    // Установка уровней предупреждения
    m_adIndicator->setAlertLevel(100, Qt::red);
    m_tkIndicator->setAlertLevel(100, Qt::red);
    m_stIndicator->setAlertLevel(100, Qt::red);

    layout->addWidget(m_adIndicator);
    layout->addWidget(m_tkIndicator);
    layout->addWidget(m_stIndicator);
    layout->addStretch();

    return group;
}

void MonitorWidget::setDiscreteMonitor(DiscreteInputMonitor* monitor) {
    if (monitor) {
        monitor->setDiscreteLabels(m_discreteLabels);
        monitor->setCommandLabels(m_commandLabels);
    }
}

void MonitorWidget::setAnalogMonitor(AnalogValueMonitor* monitor) {
    if (monitor) {
        monitor->setIndicators(m_adIndicator, m_tkIndicator, m_stIndicator);
    }
}

QWidget* MonitorWidget::createControlButtonsWidget() {
    QGroupBox* group = new QGroupBox("Управление запуском");
    QHBoxLayout* layout = new QHBoxLayout(group);

    // Создаем только 2 кнопки управления
    m_actionButton1 = new QPushButton("Проверка готовности");
    m_actionButton2 = new QPushButton("Выход");

    // Настраиваем размеры кнопок
    m_actionButton1->setMinimumHeight(40);
    m_actionButton2->setMinimumHeight(40);

    // Настраиваем стили
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

    // Начальные стили для состояния 1
    m_actionButton1->setStyleSheet(buttonStyle + "background-color: #3498db;");
    m_actionButton2->setStyleSheet(buttonStyle + "background-color: #e74c3c;");

    // Начальное состояние кнопок
    m_actionButton1->setEnabled(true);
    m_actionButton2->setEnabled(true);

    // Добавляем кнопки в layout
    layout->addWidget(m_actionButton1);
    layout->addWidget(m_actionButton2);

    return group;
}

QWidget* MonitorWidget::createMonitorLogWidget() {
    QGroupBox* group = new QGroupBox("Журнал событий мониторинга");
    QVBoxLayout* layout = new QVBoxLayout(group);

    m_logTextEdit = new QTextEdit();
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setMaximumHeight(150);
    m_logTextEdit->setPlaceholderText("Журнал событий мониторинга...");

    layout->addWidget(m_logTextEdit);
    return group;
}
