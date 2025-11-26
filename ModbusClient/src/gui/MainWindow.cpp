#include "MainWindow.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>
#include <QTimer>
#include <QLabel>
#include <QFormLayout>
#include <QDebug>
#include <QCoreApplication>
#include <QThread>
#include <QDateEdit>
#include <QComboBox>
#include <QHeaderView>
#include <QFormLayout>
#include <QMessageBox>
#include <QTableWidget>
#include "factories/WidgetFactory.h"
#include "widgets/ConnectionWidget.h"
#include "widgets/MonitorWidget.h"
#include "widgets/ChartWidget.h"
#include "core/interfaces/IModbusClient.h"
#include "data/interfaces/IDataRepository.h"
#include "export/interfaces/IExportStrategy.h"
#include "core/connection/ConnectionManager.h"
#include "monitoring/DataMonitor.h"
#include "control/ModeController.h"
#include "control/ParameterController.h"
#include "control/ControlStateMachine.h"
#include "control/ControlUIController.h"
#include "core/modbus/DeltaModbusClient.h"
#include "core/modbus/factoies/PollingConfiguratorFactory.h"
#include "core/mapping/DeltaAddressMap.h"
#include "data/database/SqliteDatabaseRepository.h"
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/DatabaseExportService.h"

MainWindow::MainWindow(IModbusClient* modbusClient,
                       IDataRepository* dataRepository,
                       IExportStrategy* exportStrategy,
                       QWidget* parent)
    : QMainWindow(parent)
    , m_modbusClient(modbusClient)
    , m_dataRepository(dataRepository)
    , m_exportStrategy(exportStrategy)
    , m_databaseRepository(nullptr)
    , m_databaseManager(nullptr)
    , m_databaseExportService(nullptr)
    , m_connectionManager(nullptr)
    , m_dataMonitor(nullptr)
    , m_modeController(nullptr)
    , m_paramController(nullptr)
    , m_controlStateMachine(nullptr)
    , m_controlUIController(nullptr)
    , m_widgetFactory(nullptr)
    , m_tabWidget(nullptr)
    , m_connectionWidget(nullptr)
    , m_monitorWidget(nullptr)
    , m_chartWidget(nullptr)
    , m_logTextEdit(nullptr)
    , m_exportButton(nullptr)
    , m_m14Status(false)
{
    // Проверяем обязательные зависимости
    if (!m_modbusClient) {
        qCritical() << "ModbusClient is null!";
        return;
    }
    // Настройка опроса после создания клиента
    PollingConfiguratorFactory::configureDefaultPolling(m_modbusClient);
    if (!m_dataRepository) {
        qCritical() << "DataRepository is null!";
        return;
    }
    if (!m_exportStrategy) {
        qCritical() << "ExportStrategy is null!";
        return;
    }
    try {
        setupUI();
        // Create managers
        m_connectionManager = new ConnectionManager(m_modbusClient, this);
        m_dataMonitor = new DataMonitor(m_modbusClient, m_dataRepository, this);
        m_modeController = new ModeController(m_modbusClient, this);
        m_paramController = new ParameterController(m_modbusClient, this);
        // Инициализация БД ДО создания state machine
        setupDatabase();
        // СОЗДАЕМ STATE MACHINE
        m_controlStateMachine = new ControlStateMachine(m_modbusClient, this);
        m_controlUIController = new ControlUIController(m_controlStateMachine, this);
        // Проверяем создание контроллеров
        if (!m_controlStateMachine || !m_controlUIController) {
            qCritical() << "State machine or UI controller creation failed!";
            return;
        }
        // Setup controllers with UI elements
        m_modeController->setModeButtons(
            m_widgetFactory->modeButton1(),
            m_widgetFactory->modeButton2(),
            m_widgetFactory->modeButton3(),
            m_widgetFactory->modeButton4(),
            m_widgetFactory->modeButton5(),
            m_widgetFactory->stopButton()
            );
        m_paramController->setParameterComboBox(m_widgetFactory->parameterComboBox());
        m_paramController->setAddressEdit(m_widgetFactory->addressEdit());
        m_paramController->setAddButton(m_widgetFactory->addParameterButton());
        // НАСТРАИВАЕМ НОВЫЙ UI КОНТРОЛЛЕР
        if (m_controlUIController && m_monitorWidget) {
            // Проверяем, что кнопки существуют
            if (m_monitorWidget->actionButton1() && m_monitorWidget->actionButton2()) {
                if (!m_controlUIController->isInitialized())
                {
                    m_controlUIController->setControlButtons(
                        m_monitorWidget->actionButton1(),
                        m_monitorWidget->actionButton2()
                    );
                } else {
                    qDebug() << "ControlUIController already initialized, skipping";
                }
            } else {
                qCritical() << "MonitorWidget buttons are not available";
            }
        } else {
            if (!m_controlUIController) qCritical() << "ControlUIController is null";
            if (!m_monitorWidget) qCritical() << "MonitorWidget is null";
        }
        // Setup monitors
        m_monitorWidget->setDiscreteMonitor(m_dataMonitor->discreteMonitor());
        m_monitorWidget->setAnalogMonitor(m_dataMonitor->analogMonitor());

        connectSignals();

        setWindowTitle("Delta AS332T Modbus Client");
        showMaximized();
    }
    catch (const std::exception& e) {
        qCritical() << "Exception in MainWindow constructor:" << e.what();
        QMessageBox::critical(nullptr, "Error",
                              QString("Failed to initialize: %1").arg(e.what()));
    }
}

MainWindow::~MainWindow() {
    // Останавливаем мониторинг первым
    if (m_dataMonitor) {
        m_dataMonitor->stopMonitoring();
    }

    // Если подключены, сбрасываем все статусные регистры
    if (m_connectionManager && m_connectionManager->isConnected()) {
        resetAllRegisters();
    }

    // Затем отключаем соединение
    if (m_connectionManager) {
        m_connectionManager->disconnectFromDevice();
    }

    // Даем время на завершение операций
    QThread::msleep(50);
    QCoreApplication::processEvents();
}

void MainWindow::setupUI() {
    m_widgetFactory = new WidgetFactory(this);
    m_tabWidget = new QTabWidget(this);

    createControlTab();
    createMonitorTab();
    createHistoryTab();

    setCentralWidget(m_tabWidget);
}

void MainWindow::createControlTab() {
    QWidget* controlTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(controlTab);

    // Connection widget
    m_connectionWidget = m_widgetFactory->createConnectionWidget();
    layout->addWidget(m_connectionWidget);

    // Mode control
    layout->addWidget(m_widgetFactory->createModeControlWidget());

    // Log widget
    QGroupBox* logGroup = new QGroupBox("Журнал событий");
    QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
    m_logTextEdit = m_widgetFactory->createLogWidget();
    logLayout->addWidget(m_logTextEdit);
    layout->addWidget(logGroup);

    layout->addStretch();

    m_tabWidget->addTab(controlTab, "Управление");
}

void MainWindow::createMonitorTab() {
    QWidget* monitorTab = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(monitorTab);

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

    // Создаем кнопку экспорта
    m_exportButton = new QPushButton("Экспорт графика");
    m_exportButton->setMinimumHeight(35);

    rightLayout->addWidget(m_chartWidget);
    rightLayout->addWidget(m_exportButton);

    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({400, 800});

    layout->addWidget(splitter);

    m_tabWidget->addTab(monitorTab, "Мониторинг и графики");
}

void MainWindow::setupDatabase() {
    // Создаем репозиторий БД
    m_databaseRepository = new SqliteDatabaseRepository(this);
    if (!m_databaseRepository->initializeDatabase()) {
        onLogMessage("Ошибка: не удалось инициализировать базу данных");
        return;
    }

    // Создаем асинхронный менеджер
    m_databaseManager = new DatabaseAsyncManager(m_databaseRepository/*, this*/);
    m_databaseManager->start();

    // Создаем сервис экспорта
    m_databaseExportService = new DatabaseExportService(m_databaseRepository, this);

    // Подключаем сигналы
    connect(m_databaseManager, &DatabaseAsyncManager::testSessionSaved,
            this, &MainWindow::onDatabaseSessionSaved);
    connect(m_databaseManager, &DatabaseAsyncManager::dataPointsLoaded,
            this, &MainWindow::onHistoricalDataLoaded);
    connect(m_databaseManager, &DatabaseAsyncManager::testSessionsLoaded,
            this, &MainWindow::onTestSessionsLoaded);
    connect(m_databaseExportService, &DatabaseExportService::exportCompleted,
            this, &MainWindow::onLogMessage);
    connect(m_databaseExportService, &DatabaseExportService::exportFailed,
            this, &MainWindow::onErrorOccurred);

    onLogMessage("Система базы данных инициализирована");
}

void MainWindow::createHistoryTab() {
    QWidget* historyTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(historyTab);

    // Заголовок
    QLabel* titleLabel = new QLabel("История тестов");
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);

    // Панель поиска
    QGroupBox* searchGroup = new QGroupBox("Поиск тестовых сессий");
    QFormLayout* searchLayout = new QFormLayout(searchGroup);

    m_fromDateEdit = new QDateEdit(QDate::currentDate().addDays(-30));
    m_fromDateEdit->setCalendarPopup(true);
    m_toDateEdit = new QDateEdit(QDate::currentDate());
    m_toDateEdit->setCalendarPopup(true);

    m_testTypeCombo = new QComboBox();
    m_testTypeCombo->addItem("Все типы", "");
    m_testTypeCombo->addItem("Расконсервация/Консервация", "Расконсервация/Консервация");
    m_testTypeCombo->addItem("Холодная прокрутка турбостартера", "Холодная прокрутка турбостартера");
    m_testTypeCombo->addItem("Регулировка мощности, замер параметров", "Регулировка мощности, замер параметров");
    m_testTypeCombo->addItem("Холодная прокрутка основного двигателя", "Холодная прокрутка основного двигателя");
    m_testTypeCombo->addItem("Имитация запуска основного двигателя", "Имитация запуска основного двигателя");

    m_loadSessionsButton = new QPushButton("Загрузить сессии");
    m_exportCsvButton = new QPushButton("Экспорт в CSV");
    m_exportImageButton = new QPushButton("Экспорт в изображение");
    m_loadDataButton = new QPushButton("Загрузить данные");

    m_exportCsvButton->setEnabled(false);
    m_exportImageButton->setEnabled(false);
    m_loadDataButton->setEnabled(false);

    searchLayout->addRow("Период с:", m_fromDateEdit);
    searchLayout->addRow("по:", m_toDateEdit);
    searchLayout->addRow("Тип теста:", m_testTypeCombo);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loadSessionsButton);
    buttonLayout->addWidget(m_loadDataButton);
    buttonLayout->addWidget(m_exportCsvButton);
    buttonLayout->addWidget(m_exportImageButton);

    searchLayout->addRow(buttonLayout);
    layout->addWidget(searchGroup);

    // Таблица сессий
    QGroupBox* sessionsGroup = new QGroupBox("Тестовые сессии");
    QVBoxLayout* sessionsLayout = new QVBoxLayout(sessionsGroup);

    m_sessionsTable = new QTableWidget();
    m_sessionsTable->setColumnCount(5);
    m_sessionsTable->setHorizontalHeaderLabels({
        "ID", "Тип теста", "Начало", "Окончание", "Длительность"
    });
    m_sessionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sessionsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_sessionsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Настройка ширины колонок
    m_sessionsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_sessionsTable->setColumnWidth(0, 50);
    m_sessionsTable->setColumnWidth(2, 150);
    m_sessionsTable->setColumnWidth(3, 150);
    m_sessionsTable->setColumnWidth(4, 100);

    sessionsLayout->addWidget(m_sessionsTable);
    layout->addWidget(sessionsGroup);

    // Группа для отображения исторических данных
    QGroupBox* dataGroup = new QGroupBox("Исторические данные");
    QVBoxLayout* dataLayout = new QVBoxLayout(dataGroup);

    // Здесь можно добавить специальный виджет для отображения исторических графиков
    // или использовать существующий ChartWidget с модификациями

    QLabel* dataInfoLabel = new QLabel("Выберите сессию из таблицы для просмотра данных");
    dataInfoLabel->setAlignment(Qt::AlignCenter);
    dataLayout->addWidget(dataInfoLabel);

    layout->addWidget(dataGroup);
    layout->addStretch();

    m_tabWidget->addTab(historyTab, "История");

    // Подключение сигналов
    connect(m_loadSessionsButton, &QPushButton::clicked, this, &MainWindow::loadHistoricalSessions);
    connect(m_loadDataButton, &QPushButton::clicked, this, &MainWindow::onLoadHistoricalData);
    connect(m_exportCsvButton, &QPushButton::clicked, this, &MainWindow::onExportSessionToCsv);
    connect(m_exportImageButton, &QPushButton::clicked, this, &MainWindow::onExportSessionToImage);
    connect(m_sessionsTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::onSessionTableSelectionChanged);
}

void MainWindow::loadHistoricalSessions() {
    if (!m_databaseManager) {
        onLogMessage("Ошибка: менеджер БД не инициализирован");
        return;
    }

    QDateTime from = QDateTime(m_fromDateEdit->date(), QTime(0, 0, 0));
    QDateTime to = QDateTime(m_toDateEdit->date(), QTime(23, 59, 59));
    QString testType = m_testTypeCombo->currentData().toString();

    onLogMessage(QString("Загрузка сессий с %1 по %2").arg(
        from.toString("dd.MM.yyyy"), to.toString("dd.MM.yyyy")));

    m_databaseManager->loadTestSessions(from, to, testType);
}

void MainWindow::displayHistoricalData(const QVector<DataPointRecord>& points) {
    // Группируем точки по параметрам для анализа
    QMap<QString, int> pointsByParameter;
    QDateTime minTime, maxTime;

    for (const auto& point : points) {
        pointsByParameter[point.parameter]++;

        if (!minTime.isValid() || point.timestamp < minTime) {
            minTime = point.timestamp;
        }
        if (!maxTime.isValid() || point.timestamp > maxTime) {
            maxTime = point.timestamp;
        }
    }

    // Выводим статистику
    QStringList stats;
    for (auto it = pointsByParameter.begin(); it != pointsByParameter.end(); ++it) {
        stats.append(QString("%1: %2 точек").arg(it.key()).arg(it.value()));
    }

    onLogMessage(QString("Период данных: %1 - %2")
                .arg(minTime.toString("dd.MM.yyyy HH:mm:ss"))
                .arg(maxTime.toString("dd.MM.yyyy HH:mm:ss")));
    onLogMessage(QString("Распределение по параметрам: %1").arg(stats.join(", ")));
}

void MainWindow::updateSessionsTable(const QVector<TestSession>& sessions) {
    m_sessionsTable->setRowCount(sessions.size());

    for (int i = 0; i < sessions.size(); ++i) {
        const TestSession& session = sessions[i];

        // ID
        QTableWidgetItem* idItem = new QTableWidgetItem(QString::number(session.id));
        idItem->setData(Qt::UserRole, session.id);
        m_sessionsTable->setItem(i, 0, idItem);

        // Тип теста
        m_sessionsTable->setItem(i, 1, new QTableWidgetItem(session.testType));

        // Время начала
        m_sessionsTable->setItem(i, 2, new QTableWidgetItem(
            session.startTime.toString("dd.MM.yyyy HH:mm:ss")));

        // Время окончания
        QString endTimeStr = session.endTime.isValid() ?
            session.endTime.toString("dd.MM.yyyy HH:mm:ss") : "Не завершена";
        m_sessionsTable->setItem(i, 3, new QTableWidgetItem(endTimeStr));

        // Длительность
        QString durationStr = "N/A";
        if (session.endTime.isValid() && session.startTime.isValid()) {
            qint64 duration = session.startTime.secsTo(session.endTime);
            if (duration > 0) {
                int hours = duration / 3600;
                int minutes = (duration % 3600) / 60;
                int seconds = duration % 60;
                durationStr = QString("%1:%2:%3")
                    .arg(hours, 2, 10, QChar('0'))
                    .arg(minutes, 2, 10, QChar('0'))
                    .arg(seconds, 2, 10, QChar('0'));
            }
        }
        m_sessionsTable->setItem(i, 4, new QTableWidgetItem(durationStr));
    }
}

void MainWindow::clearHistoricalData() {
    m_sessionsTable->setRowCount(0);
    m_currentSelectedSessionId = -1;
    m_exportCsvButton->setEnabled(false);
    m_exportImageButton->setEnabled(false);
    m_loadDataButton->setEnabled(false);
}

void MainWindow::onDatabaseSessionSaved(int sessionId) {
    onLogMessage(QString("Test session saved to database with ID: %1").arg(sessionId));
}

void MainWindow::onHistoricalDataLoaded(const QVector<DataPointRecord>& points) {
    if (!points.isEmpty()) {
        onLogMessage(QString("Загружено %1 точек данных").arg(points.size()));
        displayHistoricalData(points);

        // Переключаем на вкладку мониторинга для просмотра графиков
        m_tabWidget->setCurrentIndex(1); // Вкладка мониторинга

        // Здесь нужно обновить ChartWidget для отображения исторических данных
        if (m_chartWidget) {
            // Нужно добавить метод в ChartWidget для загрузки исторических данных
             m_chartWidget->loadHistoricalData(points);
        }
    } else {
        QMessageBox::information(this, "Информация", "Для выбранной сессии нет данных");
    }
}

void MainWindow::onTestSessionsLoaded(const QVector<TestSession>& sessions) {
    updateSessionsTable(sessions);
    onLogMessage(QString("Загружено %1 тестовых сессий").arg(sessions.size()));
}

void MainWindow::onLoadHistoricalData() {
    if (m_currentSelectedSessionId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    if (!m_databaseManager) {
        QMessageBox::critical(this, "Ошибка", "Менеджер БД не инициализирован");
        return;
    }

    onLogMessage(QString("Загрузка данных для сессии ID: %1").arg(m_currentSelectedSessionId));
    m_databaseManager->loadDataPoints(m_currentSelectedSessionId);
}

void MainWindow::onSessionTableSelectionChanged() {
    QList<QTableWidgetItem*> selectedItems = m_sessionsTable->selectedItems();
    bool hasSelection = !selectedItems.isEmpty();

    m_exportCsvButton->setEnabled(hasSelection);
    m_exportImageButton->setEnabled(hasSelection);
    m_loadDataButton->setEnabled(hasSelection);

    if (hasSelection) {
        int row = selectedItems.first()->row();
        QTableWidgetItem* idItem = m_sessionsTable->item(row, 0);
        m_currentSelectedSessionId = idItem->data(Qt::UserRole).toInt();

        onLogMessage(QString("Выбрана сессия ID: %1").arg(m_currentSelectedSessionId));
    } else {
        m_currentSelectedSessionId = -1;
    }
}

void MainWindow::onExportSessionToImage() {
    if (m_currentSelectedSessionId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this, "Экспорт в изображение",
        QString("session_%1.png").arg(m_currentSelectedSessionId),
        "PNG Files (*.png);;JPEG Files (*.jpg)");

    if (!filename.isEmpty()) {
        onLogMessage(QString("Экспорт сессии %1 в изображение: %2").arg(m_currentSelectedSessionId).arg(filename));

        if (m_databaseExportService && m_exportStrategy) {
            m_databaseExportService->exportSessionToImage(
                m_currentSelectedSessionId, filename, m_exportStrategy);
        }
    }
}

void MainWindow::onExportSessionToCsv() {
    if (m_currentSelectedSessionId <= 0) {
        QMessageBox::warning(this, "Ошибка", "Не выбрана тестовая сессия");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this, "Экспорт в CSV",
        QString("session_%1.csv").arg(m_currentSelectedSessionId),
        "CSV Files (*.csv)");

    if (!filename.isEmpty()) {
        onLogMessage(QString("Экспорт сессии %1 в CSV: %2").arg(m_currentSelectedSessionId).arg(filename));

        if (m_databaseExportService) {
            m_databaseExportService->exportSessionToCsv(m_currentSelectedSessionId, filename);
        }
    }
}


void MainWindow::connectSignals() {
    // Connection widget
    connect(m_connectionWidget, &ConnectionWidget::connectRequested, this, &MainWindow::onConnectRequested);
    connect(m_connectionWidget, &ConnectionWidget::disconnectRequested, this, &MainWindow::onDisconnectRequested);

    // Connection manager
    connect(m_connectionManager, &ConnectionManager::logMessage, this, &MainWindow::onLogMessage);
    connect(m_connectionManager, &ConnectionManager::errorOccurred, this, &MainWindow::onErrorOccurred);
    connect(m_connectionManager, &ConnectionManager::connectionStatusChanged, this, &MainWindow::onConnectionStatusChanged);

    // Data monitor
    connect(m_dataMonitor, &DataMonitor::logMessage, this, &MainWindow::onLogMessage);

    // Mode controller
    connect(m_modeController, &ModeController::logMessage, this, &MainWindow::onLogMessage);

    // АВТОМАТИЧЕСКОЕ ПЕРЕКЛЮЧЕНИЕ НА ВКЛАДКУ МОНИТОРИНГА ПРИ ЗАПУСКЕ ТЕСТА
//    connect(m_modeController, &ModeController::testStarted,
//            this, [this](const QString& mode) {
//                m_tabWidget->setCurrentIndex(1);
//                onLogMessage(QString("Автоматическое переключение на вкладку мониторинга для режима: %1").arg(mode));
//            });

    connect(m_controlStateMachine, &ControlStateMachine::stateChanged, m_controlUIController, &ControlUIController::onStateChanged);

    // Control state machine signals
    connect(m_controlStateMachine, &ControlStateMachine::registerWriteVerified,
                    this, [this](const QString& registerName, quint16 value, bool success) {
                        QString status = success ? "✓ УСПЕХ" : "✗ ОШИБКА";
                        QString message = QString("[%1] Верификация записи %2=%3 - %4")
                                             .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                                             .arg(registerName)
                                             .arg(value)
                                             .arg(status);
                        onLogMessage(message);

                        if (!success) {
                            onLogMessage("⚠️ Проверьте подключение к устройству");
                        }
                    });

    connect(m_controlStateMachine, &ControlStateMachine::logMessage, this, &MainWindow::onLogMessage);
    connect(m_controlStateMachine, &ControlStateMachine::modeSelectionRequested,
            this, [this]() {
                m_tabWidget->setCurrentIndex(0); // Переключаем на вкладку выбора режима
                onLogMessage("Переключение на вкладку выбора режима");
            });

    connect(m_controlStateMachine, &ControlStateMachine::exitRequested,
               this, [this]() {
           // Переключаем на вкладку управления при выходе
           if (m_tabWidget) {
               m_tabWidget->setCurrentIndex(0);
               onLogMessage("Выход в меню выбора режима");
           }
       });

    connect(m_controlStateMachine, &ControlStateMachine::startChartRecording, m_chartWidget, &ChartWidget::startTestRecording);
    connect(m_controlStateMachine, &ControlStateMachine::stopChartRecording, m_chartWidget, &ChartWidget::stopTestRecording);
    connect(m_controlStateMachine, &ControlStateMachine::stopCurrentTest, m_modeController, &ModeController::stopTest);

    // Parameter controller
    connect(m_paramController, &ParameterController::logMessage, this, &MainWindow::onLogMessage);
    connect(m_paramController, &ParameterController::parameterSelected, this, &MainWindow::onParameterSelected);

    // Export button
    connect(m_exportButton, &QPushButton::clicked, this, &MainWindow::onExportClicked);

    // СИНХРОНИЗАЦИЯ ЖУРНАЛОВ МЕЖДУ ВКЛАДКАМИ
    connect(this, &MainWindow::logMessageAdded,
            this, [this](const QString& message) {
                // Добавляем сообщение в оба журнала
                if (m_logTextEdit) {
                    m_logTextEdit->append(message);
                }
                if (m_monitorWidget && m_monitorWidget->logTextEdit()) {
                    m_monitorWidget->logTextEdit()->append(message);
                }
            });

    // ПОДКЛЮЧАЕМ МОНИТОРИНГ СТАТУСНЫХ РЕГИСТРОВ
    connect(m_modbusClient, &IModbusClient::registerReadCompleted,
            this, [this](QModbusDataUnit::RegisterType type, quint16 address, quint16 value) {
                if (type == QModbusDataUnit::Coils) {
                    if (address == DeltaAS332T::Addresses::M11_READY_STATUS) {
                        m_controlStateMachine->onM11StatusChanged(value > 0);
                    } else if (address == DeltaAS332T::Addresses::M12_START_STATUS) {
                        m_controlStateMachine->onM12StatusChanged(value > 0);
                    } else if (address == DeltaAS332T::Addresses::M14_COMPLETE_STATUS) {
                        m_m14Status = (value > 0);
                        checkCompletionCondition();
                        // Проверяем M0 и M14 для завершения
                        // M0 нужно читать отдельно
                    } else if (address == DeltaAS332T::Addresses::M0_STOP_STATUS) {
                        m_m0Status = (value == 1);
                        checkCompletionCondition();
                    }
                }
    });
}

void MainWindow::checkCompletionCondition() {
    if (m_controlStateMachine && m_controlStateMachine->currentState() == ControlStateMachine::STATE_STOP) {
        // M0 == 1 && M14 == 1 означает завершение
        bool completed = m_m0Status && m_m14Status;
        m_controlStateMachine->onM0M14StatusChanged(completed);
    }
}

void MainWindow::resetAllRegisters() {
    if (!m_modbusClient || !m_modbusClient->isConnected()) {
        return;
    }

    try {
        // Сбрасываем регистр режима D0 в 0
        m_modbusClient->writeRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::D0_MODE_REGISTER, 0);

        // Сбрасываем все M-регистры статуса
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS, 0);
        // Сбрасываем все M-регистры управления
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M1_READY_CHECK, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M2_START, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M3_STOP, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M4_RESTART, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M5_INTERRUPT, 0);
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M6_EXIT, 0);

        // Сбрасываем командные выходы K1-K6
        for (int i = 0; i < 6; ++i) {
            quint16 address = DeltaAS332T::Addresses::K1 + i;
            // Пропускаем K6, так как он имеет другой адрес
            if (address <= DeltaAS332T::Addresses::K5) {
                m_modbusClient->writeRegister(QModbusDataUnit::Coils, address, 0);
            }
        }
        m_modbusClient->writeRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K6, 0);

        onLogMessage(QString("[%1] Все регистры сброшены при закрытии приложения")
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

    } catch (const std::exception& e) {
        qCritical() << "Exception while resetting registers:" << e.what();
    }
}



void MainWindow::onConnectRequested(const QString& address,
                                    quint16 serverPort,
                                    quint16 clientPort) {
    // Set local port for Delta client
    if (auto deltaClient = qobject_cast<DeltaModbusClient*>(m_modbusClient)) {
        deltaClient->setLocalPort(clientPort);
    }

    m_connectionManager->connectToDevice(address, serverPort);
}

void MainWindow::onDisconnectRequested() {
    m_connectionManager->disconnectFromDevice();
}

void MainWindow::onExportClicked() {
    QString filename = QFileDialog::getSaveFileName(
        this, "Экспорт графика", "chart.png", "PNG Files (*.png)");

    if (!filename.isEmpty()) {
        if (m_exportStrategy->exportWidget(m_chartWidget, filename)) {
            onLogMessage(QString("График экспортирован в %1").arg(filename));
        } else {
            QMessageBox::warning(this, "Ошибка экспорта",
                               "Не удалось экспортировать график");
        }
    }
}

void MainWindow::onLogMessage(const QString& message) {
    emit logMessageAdded(message);
}

void MainWindow::onErrorOccurred(const QString& error) {
    onLogMessage(error);
    QMessageBox::warning(this, "Ошибка", error);
}

void MainWindow::onConnectionStatusChanged(const QString& status) {
    m_connectionWidget->setStatus(status);
    m_connectionWidget->setConnected(status == "ПОДКЛЮЧЕН");

    bool connected = (status == "ПОДКЛЮЧЕН");
    m_modeController->setConnectionState(connected);

    if (connected) {
        m_dataMonitor->startMonitoring();
        // updateControlButtons();
    } else {
        m_dataMonitor->stopMonitoring();
        // updateControlButtons();
    }
}

void MainWindow::onParameterSelected(const QString& parameter) {
    m_chartWidget->setParameter(parameter);
}


