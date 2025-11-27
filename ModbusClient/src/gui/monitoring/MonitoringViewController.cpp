#include "MonitoringViewController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QPushButton>
#include <QTextEdit>
#include <QDebug>

MonitoringViewController::MonitoringViewController(DataMonitor* dataMonitor,
                                                   ControlStateMachine* controlStateMachine,
                                                   ModeController* modeController,
                                                   IDataRepository* dataRepository,
                                                   QObject* parent)
    : QObject(parent)
    , m_dataMonitor(dataMonitor)
    , m_controlStateMachine(controlStateMachine)
    , m_modeController(modeController)
    , m_dataRepository(dataRepository)
    , m_controlUIController(nullptr)
    , m_mainWidget(nullptr)
    , m_monitorWidget(nullptr)
    , m_chartWidget(nullptr)
    , m_widgetFactory(nullptr)
    , m_exportButton(nullptr)
{
    setupUI();
    setupConnections();
}

QWidget* MonitoringViewController::getWidget() {
    return m_mainWidget;
}

void MonitoringViewController::setupUI() {
    m_mainWidget = new QWidget();
    m_widgetFactory = new WidgetFactory(m_mainWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(m_mainWidget);

    QSplitter* splitter = new QSplitter(Qt::Horizontal);

    // Левая панель - monitoring
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    m_monitorWidget = m_widgetFactory->createMonitorWidget();
    leftLayout->addWidget(m_monitorWidget);
    leftLayout->addStretch();

    // Правая панель - chart
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

    m_chartWidget = m_widgetFactory->createChartWidget(m_dataRepository);
    m_exportButton = new QPushButton("Экспорт графика");
    m_exportButton->setMinimumHeight(35);

    rightLayout->addWidget(m_chartWidget);
    rightLayout->addWidget(m_exportButton);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({400, 800});

    mainLayout->addWidget(splitter);
}

void MonitoringViewController::setupConnections() {
    if (!m_monitorWidget || !m_controlStateMachine) {
        qCritical() << "MonitoringViewController: Critical components are null!";
        return;
    }

    // Создаем UI контроллер для управления кнопками
    m_controlUIController = new ControlUIController(m_controlStateMachine, this);
    m_controlUIController->setControlButtons(
        m_monitorWidget->actionButton1(),
        m_monitorWidget->actionButton2()
    );

    // Подключаем сигналы мониторинга
    if (m_dataMonitor && m_monitorWidget) {
        m_monitorWidget->setDiscreteMonitor(m_dataMonitor->discreteMonitor());
        m_monitorWidget->setAnalogMonitor(m_dataMonitor->analogMonitor());
    }

    // Подключаем сигналы режимов
    if (m_modeController) {
        connect(m_modeController, &ModeController::logMessage,
                this, &MonitoringViewController::onLogMessage);
        connect(m_modeController, &ModeController::testStarted,
                this, &MonitoringViewController::onTestStarted);
        connect(m_modeController, &ModeController::testStopped,
                this, &MonitoringViewController::onTestStopped);
    }

    // Подключаем экспорт
    connect(m_exportButton, &QPushButton::clicked,
            this, &MonitoringViewController::onExportClicked);

    // Подключаем state machine к графику
    if (m_controlStateMachine && m_chartWidget) {
        connect(m_controlStateMachine, &ControlStateMachine::startChartRecording,
                m_chartWidget, &ChartWidget::startTestRecording);
        connect(m_controlStateMachine, &ControlStateMachine::stopChartRecording,
                m_chartWidget, &ChartWidget::stopTestRecording);
    }

    qDebug() << "MonitoringViewController: All connections established";
}

void MonitoringViewController::startMonitoring() {
    if (m_dataMonitor) {
        m_dataMonitor->startMonitoring();
        qDebug() << "MonitoringViewController: Monitoring started";
    }
}

void MonitoringViewController::stopMonitoring() {
    if (m_dataMonitor) {
        m_dataMonitor->stopMonitoring();
        qDebug() << "MonitoringViewController: Monitoring stopped";
    }
}

void MonitoringViewController::setRecording(bool recording) {
    if (recording) {
        if (m_chartWidget) {
            m_chartWidget->startTestRecording();
        }
    } else {
        if (m_chartWidget) {
            m_chartWidget->stopTestRecording();
        }
    }
}

void MonitoringViewController::onLogMessage(const QString& message) {
    qDebug() << "Monitoring:" << message;

    // Если есть лог в монитор виджете, добавляем туда
    if (m_monitorWidget && m_monitorWidget->logTextEdit()) {
        m_monitorWidget->logTextEdit()->append(message);
    }
}

void MonitoringViewController::onTestStarted(const QString& mode) {
//    setRecording(true);
    onLogMessage(QString("Тест запущен: %1").arg(mode));
}

void MonitoringViewController::onTestStopped() {
    setRecording(false);
    onLogMessage("Тест остановлен");
}

void MonitoringViewController::onExportClicked() {
    if (m_chartWidget) {
        onLogMessage("Экспорт графика запрошен");
        emit exportRequested();
    }
}
