#pragma once
#include <QMainWindow>
#include <QSplitter>
#include <QCloseEvent>

class QTabWidget;
class ConnectionViewController;
class ModeSelectionViewController;
class MonitoringViewController;
class DatabaseViewController;
class IDataRepository;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

    void setupUI(ConnectionViewController* connectionVC,
                 ModeSelectionViewController* modeSelectionVC,
                 MonitoringViewController* monitoringVC,
                 DatabaseViewController* databaseVC);

    void setDataRepository(IDataRepository* repository) { m_dataRepository = repository; }

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupConnections();

    QSplitter* m_mainSplitter;
    QTabWidget* m_tabWidget;

    ConnectionViewController* m_connectionVC;
    ModeSelectionViewController* m_modeSelectionVC;
    MonitoringViewController* m_monitoringVC;
    DatabaseViewController* m_databaseVC;
    IDataRepository* m_dataRepository;
};
