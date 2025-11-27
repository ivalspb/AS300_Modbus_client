
#pragma once
#include "control/ModeController.h"
#include "../factories/WidgetFactory.h"
#include "core/connection/ConnectionManager.h"
#include <QObject>
#include <QWidget>

class ModeSelectionViewController : public QObject {
    Q_OBJECT
public:
    explicit ModeSelectionViewController(ModeController* modeController,
                                         QObject* parent = nullptr);
    ~ModeSelectionViewController() override = default;

    QWidget* getWidget();

    void setConnectionState(bool connected);

public slots:
    void onTestStarted(const QString& mode);
    void onTestStopped();
    void onConnectionStateChanged(bool connected);

private:
    void setupUI();
    void setupConnections();
    void updateButtonStates();
    void setActiveModeButton(QPushButton* activeButton);

    ModeController* m_modeController;
    QWidget* m_mainWidget;
    WidgetFactory* m_widgetFactory;

    // Кнопки выбора режима
    QPushButton* m_modeButton1;
    QPushButton* m_modeButton2;
    QPushButton* m_modeButton3;
    QPushButton* m_modeButton4;
    QPushButton* m_modeButton5;
    QPushButton* m_stopButton;

    bool m_connected;
    bool m_testRunning;
    QString m_currentMode;
};
