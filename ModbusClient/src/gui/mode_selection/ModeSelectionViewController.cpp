#include "ModeSelectionViewController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QDebug>

ModeSelectionViewController::ModeSelectionViewController(ModeController* modeController,
                                                         QObject* parent)
    : QObject(parent)
    , m_modeController(modeController)
    , m_mainWidget(nullptr)
    , m_widgetFactory(nullptr)
    , m_modeButton1(nullptr)
    , m_modeButton2(nullptr)
    , m_modeButton3(nullptr)
    , m_modeButton4(nullptr)
    , m_modeButton5(nullptr)
    , m_stopButton(nullptr)
    , m_connected(false)
    , m_testRunning(false)
{
    setupUI();
    setupConnections();
}

QWidget* ModeSelectionViewController::getWidget() {
    return m_mainWidget;
}

void ModeSelectionViewController::setupUI() {
    m_mainWidget = new QWidget();
    m_widgetFactory = new WidgetFactory(m_mainWidget);

    QVBoxLayout* mainLayout = new QVBoxLayout(m_mainWidget);

    // Группа выбора режима
    QGroupBox* modeGroup = new QGroupBox("Выбор режима работы");
    QGridLayout* modeLayout = new QGridLayout(modeGroup);

    // Создаем кнопки выбора режима
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

    // Располагаем кнопки в сетке 2x3
    modeLayout->addWidget(m_modeButton1, 0, 0);
    modeLayout->addWidget(m_modeButton2, 0, 1);
    modeLayout->addWidget(m_modeButton3, 0, 2);
    modeLayout->addWidget(m_modeButton4, 1, 0);
    modeLayout->addWidget(m_modeButton5, 1, 1);
    modeLayout->addWidget(m_stopButton, 1, 2);

    // Устанавливаем растягивание
    modeLayout->setColumnStretch(0, 1);
    modeLayout->setColumnStretch(1, 1);
    modeLayout->setColumnStretch(2, 1);
    modeLayout->setRowStretch(0, 1);
    modeLayout->setRowStretch(1, 1);

    mainLayout->addWidget(modeGroup);
    mainLayout->addStretch();

    // Инициализируем состояние кнопок
    updateButtonStates();
}

void ModeSelectionViewController::setupConnections() {
    if (!m_modeController) {
        qWarning() << "ModeSelectionViewController: ModeController is null!";
        return;
    }

    // Подключаем кнопки НАПРЯМУЮ к ModeController
    connect(m_modeButton1, &QPushButton::clicked, m_modeController, &ModeController::onMode1Clicked);
    connect(m_modeButton2, &QPushButton::clicked, m_modeController, &ModeController::onMode2Clicked);
    connect(m_modeButton3, &QPushButton::clicked, m_modeController, &ModeController::onMode3Clicked);
    connect(m_modeButton4, &QPushButton::clicked, m_modeController, &ModeController::onMode4Clicked);
    connect(m_modeButton5, &QPushButton::clicked, m_modeController, &ModeController::onMode5Clicked);
    connect(m_stopButton, &QPushButton::clicked, m_modeController, &ModeController::onStopClicked);

    // Подключаем сигналы от контроллера режимов для обновления UI
    connect(m_modeController, &ModeController::testStarted, this, &ModeSelectionViewController::onTestStarted);
    connect(m_modeController, &ModeController::testStopped, this, &ModeSelectionViewController::onTestStopped);

    qDebug() << "ModeSelectionViewController: Connections established successfully";
}

void ModeSelectionViewController::updateButtonStates() {
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

    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};

    if (!m_connected) {
        // Все кнопки серые и неактивные при отключении
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
        // Тест запущен: все кнопки режимов неактивны, стоп красная
        for (QPushButton* button : modeButtons) {
            if (button) {
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

void ModeSelectionViewController::setActiveModeButton(QPushButton* activeButton) {
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

    QList<QPushButton*> modeButtons = {m_modeButton1, m_modeButton2, m_modeButton3, m_modeButton4, m_modeButton5};
    for (QPushButton* button : modeButtons) {
        if (button && button != activeButton) {
            button->setStyleSheet(disabledStyle);
            button->setEnabled(false);
        } else if (button == activeButton) {
            button->setStyleSheet(activeGreenStyle);
            button->setEnabled(false);
        }
    }
}

void ModeSelectionViewController::setConnectionState(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        updateButtonStates();
    }
}

void ModeSelectionViewController::onTestStarted(const QString& mode) {
    m_testRunning = true;
    m_currentMode = mode;

    qDebug() << "ModeSelectionViewController: Test started:" << mode;

    // Находим активную кнопку и подсвечиваем ее
    QPushButton* activeButton = nullptr;
    if (mode == "Расконсервация/Консервация") activeButton = m_modeButton1;
    else if (mode == "Холодная прокрутка турбостартера") activeButton = m_modeButton2;
    else if (mode == "Регулировка мощности, замер параметров") activeButton = m_modeButton3;
    else if (mode == "Холодная прокрутка основного двигателя") activeButton = m_modeButton4;
    else if (mode == "Имитация запуска основного двигателя") activeButton = m_modeButton5;

    if (activeButton) {
        setActiveModeButton(activeButton);
    }

    updateButtonStates();
}

void ModeSelectionViewController::onTestStopped() {
    m_testRunning = false;
    m_currentMode.clear();
    updateButtonStates();

    qDebug() << "ModeSelectionViewController: Test stopped";
}

void ModeSelectionViewController::onConnectionStateChanged(bool connected) {
    setConnectionState(connected);
}

