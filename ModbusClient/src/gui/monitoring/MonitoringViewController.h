#pragma once
#include <QObject>
#include "monitoring/DataMonitor.h"
#include "control/ControlStateMachine.h"
#include "control/ControlUIController.h"
#include "control/ModeController.h"
#include "../widgets/MonitorWidget.h"
#include "../widgets/ChartWidget.h"
#include "../factories/WidgetFactory.h"

class MonitoringViewController : public QObject {
    Q_OBJECT
public:
    explicit MonitoringViewController(DataMonitor* dataMonitor,
                                    ControlStateMachine* controlStateMachine,
                                    ModeController* modeController,
                                    IDataRepository* dataRepository,
                                    QObject* parent = nullptr);
    ~MonitoringViewController() override = default;

    QWidget* getWidget();

    void startMonitoring();
    void stopMonitoring();
    void setRecording(bool recording);

signals:
    void exportRequested();

private slots:
    void onLogMessage(const QString& message);
    void onTestStarted(const QString& mode);
    void onTestStopped();
    void onExportClicked();

private:
    void setupUI();
    void setupConnections();

    DataMonitor* m_dataMonitor;
    ControlStateMachine* m_controlStateMachine;
    ModeController* m_modeController;
    IDataRepository* m_dataRepository;
    ControlUIController* m_controlUIController;

    QWidget* m_mainWidget;
    MonitorWidget* m_monitorWidget;
    ChartWidget* m_chartWidget;
    WidgetFactory* m_widgetFactory;
    QPushButton* m_exportButton;
};
