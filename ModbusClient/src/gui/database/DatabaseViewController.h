#pragma once
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/DatabaseExportService.h"
#include <QObject>
#include <QWidget>

class QTableWidget;
class QDateEdit;
class QComboBox;
class QPushButton;
class QLabel;

class DatabaseViewController : public QObject {
    Q_OBJECT
public:
    explicit DatabaseViewController(DatabaseAsyncManager* dbManager,
                                  DatabaseExportService* exportService,
                                  QObject* parent = nullptr);
    ~DatabaseViewController() override = default;

    // IDatabaseView interface
    QWidget* getWidget();

    void showSessions(const QVector<TestSession>& sessions);
    void showSessionData(const QVector<DataPointRecord>& points);
    void showExportProgress(int progress);
    void showMessage(const QString& message);

    int getSelectedSessionId() const;
    QDateTime getSearchFromDate() const;
    QDateTime getSearchToDate() const;
    QString getSearchTestType() const;

signals:
    // Сигналы IDatabaseView
    void loadSessionsRequested(const QDateTime& from, const QDateTime& to, const QString& testType);
    void loadSessionDataRequested(int sessionId);
    void exportToCsvRequested(int sessionId, const QString& filename);
    void exportToImageRequested(int sessionId, const QString& filename);

private slots:
    void onLoadSessionsClicked();
    void onLoadDataClicked();
    void onExportCsvClicked();
    void onExportImageClicked();
    void onSessionSelectionChanged();
    void onExportCompleted(const QString& filename);
    void onExportFailed(const QString& error);

private:
    void setupUI();
    void setupConnections();
    void updateSessionsTable(const QVector<TestSession>& sessions);
    QString formatDuration(const QDateTime& start, const QDateTime& end) const;

    DatabaseAsyncManager* m_dbManager;
    DatabaseExportService* m_exportService;
    QWidget* m_mainWidget;
    QTableWidget* m_sessionsTable;
    QDateEdit* m_fromDateEdit;
    QDateEdit* m_toDateEdit;
    QComboBox* m_testTypeCombo;
    QPushButton* m_loadSessionsButton;
    QPushButton* m_loadDataButton;
    QPushButton* m_exportCsvButton;
    QPushButton* m_exportImageButton;
    QLabel* m_statusLabel;

    int m_currentSessionId;
};
