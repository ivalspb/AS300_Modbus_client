#include <QApplication>
#include <QScopedPointer>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QDebug>
#include "gui/MainWindow.h"
#include "core/modbus/DeltaModbusClient.h"
#include "data/DataRepository.h"
#include "export/PngExportStrategy.h"
#include "data/database/SqliteDatabaseRepository.h"
#include "data/database/DatabaseAsyncManager.h"

void setupLogging() {
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    qputenv("QT_MODBUS_TCP_TIMEOUT", "5000");
    qputenv("QT_MODBUS_TCP_RETRIES", "1");
//    QLoggingCategory::setFilterRules("qt.modbus*=true");
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

    // Setup High DPI before QApplication
    setupHighDPI();

    QApplication app(argc, argv);

    // Application info
    app.setApplicationName("Modbus Client");
    app.setApplicationVersion("0.0.2");
    app.setOrganizationName("Красный Октябрь");
    app.setOrganizationDomain("koavia.ru");

    // Setup logging
    setupLogging();

    // Parse command line
    QCommandLineParser parser;
    setupCommandLine(app, parser);

    try {

        // Create dependencies using RAII
        QScopedPointer<IModbusClient> modbusClient(new DeltaModbusClient());
        QScopedPointer<IExportStrategy> exportStrategy(new PngExportStrategy());
        // Create database dependencies FIRST
        QScopedPointer<SqliteDatabaseRepository> databaseRepository(new SqliteDatabaseRepository());
        QScopedPointer<DatabaseAsyncManager> databaseManager(new DatabaseAsyncManager(databaseRepository.data()));
        // Create data repository with database manager
        QScopedPointer<IDataRepository> dataRepository(new DataRepository());

        databaseManager->start();

        // Create main window with dependency injection
        MainWindow window(modbusClient.data(),
                         dataRepository.data(),
                         exportStrategy.data());

        window.show();

        // Установим обработчик для корректного завершения
        QObject::connect(&app, &QApplication::aboutToQuit, []() {
        });

        int result = app.exec();
        return result;
    }
    catch (const std::exception& e) {
        qCritical() << "Fatal error:" << e.what();
        QMessageBox::critical(nullptr, "Fatal Error",
            QString("Application failed to start: %1").arg(e.what()));
        return 1;
    }
    catch (...) {
        qCritical() << "Unknown fatal error";
        QMessageBox::critical(nullptr, "Fatal Error",
            "Unknown error occurred during startup");
        return 1;
    }
}
