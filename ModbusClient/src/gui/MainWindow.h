#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QPushButton>

class IModbusClient;
class IDataRepository;
class IExportStrategy;
class ConnectionManager;
class DataMonitor;
class ModeController;
class ParameterController;
class ControlStateMachine;
class ControlUIController;
class ConnectionWidget;
class MonitorWidget;
class ChartWidget;
class WidgetFactory;
class QTabWidget;
class QTableWidget;
class QDateEdit;
class QComboBox;
class QTextEdit;
class QPushButton;
class DatabaseAsyncManager;
class SqliteDatabaseRepository;
class DatabaseExportService;
class DataPointRecord;
class TestSession;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(IModbusClient* modbusClient,
               IDataRepository* dataRepository,
               IExportStrategy* exportStrategy,
               QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectRequested(const QString& address, quint16 serverPort, quint16 clientPort);
    void onDisconnectRequested();
    void onExportClicked();
    void onLogMessage(const QString& message);
    void onErrorOccurred(const QString& error);
    void onConnectionStatusChanged(const QString& status);
    void onParameterSelected(const QString& parameter);
    void onDatabaseSessionSaved(int sessionId);
    void onHistoricalDataLoaded(const QVector<DataPointRecord>& points);
    void onTestSessionsLoaded(const QVector<TestSession>& sessions);
//    void onExportHistoricalData();
    void onLoadHistoricalData();
    void onSessionTableSelectionChanged();
    void onExportSessionToImage();
    void onExportSessionToCsv();

private:
    void setupUI();
    void createControlTab();
    void createMonitorTab();
    void connectSignals();
    void setupControlButtons();
    void checkCompletionCondition();
    void resetAllRegisters();
    void setupDatabase();
    void createHistoryTab();
    void loadHistoricalSessions();
    void displayHistoricalData(const QVector<DataPointRecord>& points);
    void updateSessionsTable(const QVector<TestSession>& sessions);
    void clearHistoricalData();

    // Dependencies
    IModbusClient* m_modbusClient;
    IDataRepository* m_dataRepository;
    IExportStrategy* m_exportStrategy;
    SqliteDatabaseRepository* m_databaseRepository;
    DatabaseAsyncManager* m_databaseManager;
    DatabaseExportService* m_databaseExportService;

    // Managers
    ConnectionManager* m_connectionManager;
    DataMonitor* m_dataMonitor;
    ModeController* m_modeController;
    ParameterController* m_paramController;
    ControlStateMachine* m_controlStateMachine;
    ControlUIController* m_controlUIController;

    // UI components
    WidgetFactory* m_widgetFactory;
    QTabWidget* m_tabWidget;
    ConnectionWidget* m_connectionWidget;
    MonitorWidget* m_monitorWidget;
    ChartWidget* m_chartWidget;
    QTextEdit* m_logTextEdit;
    QPushButton* m_exportButton;
    // Элементы UI для вкладки истории
    QTableWidget* m_sessionsTable;
    QDateEdit* m_fromDateEdit;
    QDateEdit* m_toDateEdit;
    QComboBox* m_testTypeCombo;
    QPushButton* m_loadSessionsButton;
    QPushButton* m_exportCsvButton;
    QPushButton* m_exportImageButton;
    QPushButton* m_loadDataButton;

    // Текущая выбранная сессия
    int m_currentSelectedSessionId;

    bool m_m14Status; // Для хранения статуса M14
    bool m_m0Status; // Для хранения статуса M0

signals:
    void logMessageAdded(const QString& message);
};
