
#include <QApplication>
#include <QScopedPointer>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QDebug>

#include "gui/mainwindow/MainWindow.h"
#include "gui/connection/ConnectionViewController.h"
#include "gui/database/DatabaseViewController.h"
#include "gui/monitoring/MonitoringViewController.h"
#include "gui/mode_selection/ModeSelectionViewController.h"

#include "core/modbus/DeltaModbusClient.h"
#include "core/connection/ConnectionManager.h"

#include "data/DataRepository.h"
#include "data/database/SqliteDatabaseRepository.h"
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/DatabaseExportService.h"

#include "monitoring/DataMonitor.h"

#include "control/ModeController.h"
#include "control/ControlStateMachine.h"
#include "control/ControlUIController.h"

#include "export/PngExportStrategy.h"

void setupLogging() {
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    qputenv("QT_MODBUS_TCP_TIMEOUT", "5000");
    qputenv("QT_MODBUS_TCP_RETRIES", "1");
}

void setupHighDPI() {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

void setupCommandLine(QApplication& app, QCommandLineParser& parser) {
    parser.setApplicationDescription("Modbus TCP Client with GUI for Delta AS332T");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption noScalingOption("no-scaling", "Disable High DPI scaling");
    parser.addOption(noScalingOption);

    parser.process(app);

    if (parser.isSet(noScalingOption)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
        QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, false);
#endif
    }
}

int main(int argc, char *argv[]) {
    setupHighDPI();

    QApplication app(argc, argv);
    app.setApplicationName("Modbus Client");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Красный Октябрь");
    app.setOrganizationDomain("koavia.ru");

    setupLogging();

    QCommandLineParser parser;
    setupCommandLine(app, parser);

    try {
        qDebug() << "=== Application Starting ===";

        // 1. Create core components
        qDebug() << "Creating Modbus client...";
        auto modbusClient = new DeltaModbusClient();
        auto exportStrategy = new PngExportStrategy();

        // 2. Create database components
        qDebug() << "Initializing database...";
        auto databaseRepository = new SqliteDatabaseRepository();
        if (!databaseRepository->initializeDatabase()) {
            qCritical() << "Failed to initialize database";
            return 1;
        }

        auto databaseManager = new DatabaseAsyncManager(databaseRepository);
        auto databaseExportService = new DatabaseExportService(databaseRepository);
        databaseManager->start();

        // 3. Create data repository with database support
        qDebug() << "Creating data repository...";
        auto dataRepository = new DataRepository(databaseManager);

        // 4. Create monitoring and control components
        qDebug() << "Creating connection and control components...";
        auto connectionManager = new ConnectionManager(modbusClient);
        auto dataMonitor = new DataMonitor(modbusClient, dataRepository);

        auto modeController = new ModeController(modbusClient);
        modeController->setDataRepository(dataRepository);
        qDebug() << "ModeController connected to DataRepository";

        // ВАЖНО: ControlStateMachine сам подключается к modbusClient для мониторинга статусов
        auto controlStateMachine = new ControlStateMachine(modbusClient);
        qDebug() << "ControlStateMachine initialized with status monitoring";

        // 5. Create view controllers
        qDebug() << "Creating view controllers...";
        auto connectionViewController = new ConnectionViewController(connectionManager);
        auto databaseViewController = new DatabaseViewController(databaseManager, databaseExportService);

        // MonitoringViewController больше НЕ нужен modbusClient
        auto monitoringViewController = new MonitoringViewController(
            dataMonitor,
            controlStateMachine,
            modeController,
            dataRepository
        );

        auto modeSelectionViewController = new ModeSelectionViewController(modeController);

        // 6. Setup connections between components
        qDebug() << "Setting up connections...";

        // Connect database signals
        QObject::connect(databaseManager, &DatabaseAsyncManager::testSessionsLoaded,
                        databaseViewController, &DatabaseViewController::showSessions);
        QObject::connect(databaseManager, &DatabaseAsyncManager::dataPointsLoaded,
                        databaseViewController, &DatabaseViewController::showSessionData);

        // Connect database view signals
        QObject::connect(databaseViewController, &DatabaseViewController::loadSessionsRequested,
                        databaseManager, &DatabaseAsyncManager::loadTestSessions);

        QObject::connect(databaseViewController, &DatabaseViewController::loadSessionDataRequested,
                        [databaseManager](int sessionId) {
                            databaseManager->loadDataPoints(sessionId, "");
                        });

        QObject::connect(databaseViewController, &DatabaseViewController::exportToCsvRequested,
                        databaseExportService, &DatabaseExportService::exportSessionToCsv);
        QObject::connect(databaseViewController, &DatabaseViewController::exportToImageRequested,
                        [databaseExportService, exportStrategy](int sessionId, const QString& filename) {
                            databaseExportService->exportSessionToImage(sessionId, filename, exportStrategy);
                        });

        // Connect connection status to mode selection
        QObject::connect(connectionManager, &ConnectionManager::connectionStatusChanged,
                        [modeSelectionViewController](const QString& status) {
                            bool connected = (status == "ПОДКЛЮЧЕН");
                            modeSelectionViewController->onConnectionStateChanged(connected);
                            qDebug() << "Connection state changed:" << connected;
                        });
        QObject::connect(connectionManager, &ConnectionManager::connectionStatusChanged,
                         [modeController](const QString& status) {
                             bool connected = (status == "ПОДКЛЮЧЕН");
                             modeController->setConnectionState(connected);
                             qDebug() << "ModeController connection state set to:" << connected;
                         });

        // Connect ControlStateMachine to ModeController for stopping tests
        QObject::connect(controlStateMachine, &ControlStateMachine::stopCurrentTest,
                        modeController, &ModeController::stopTest);

        // 7. Create and setup main window
        qDebug() << "Creating main window...";
        MainWindow mainWindow;
        mainWindow.setDataRepository(dataRepository);

        mainWindow.setupUI(connectionViewController, modeSelectionViewController,
                          monitoringViewController, databaseViewController);

        // 8. Start the application
        qDebug() << "Showing main window...";
        mainWindow.show();

        // Start monitoring
        qDebug() << "Starting monitoring...";
        monitoringViewController->startMonitoring();

        qDebug() << "=== Application Started Successfully ===";
        qDebug() << "✓ Status monitoring in ControlStateMachine";
        qDebug() << "✓ Automatic state transitions enabled";
        qDebug() << "✓ Clean architecture with proper separation of concerns";

        return app.exec();

    } catch (const std::exception& e) {
        qCritical() << "Fatal error:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Application failed to start: %1").arg(e.what()));
        return 1;
    }
}
