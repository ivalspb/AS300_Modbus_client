
#pragma once
#include "interfaces/IDataRepository.h"
#include "data/database/DatabaseAsyncManager.h"
#include "data/database/TestSession.h"
#include <QReadWriteLock>
#include <QMap>
#include <QVector>
#include <QObject>
#include <QTimer>

class DataRepository : public IDataRepository {
    Q_OBJECT
public:
    explicit DataRepository(DatabaseAsyncManager* dbManager = nullptr, QObject* parent = nullptr);
    ~DataRepository() override = default;

    void addDataPoint(const QString& parameter, double value) override;
    QVector<DataPoint> getDataPoints(const QString& parameter,
                                     const QDateTime& from = QDateTime(),
                                     const QDateTime& to = QDateTime()) const override;
    QVector<QString> getAvailableParameters() const override;
    void clearData(const QString& parameter = QString()) override;
    int getDataPointCount(const QString& parameter) const override;

    // Методы для работы с БД
    void setCurrentTestSession(const QString& testType) override;
    void saveCurrentSessionToDatabase() override;
    void finalizeSession() override; // Принудительное завершение сессии
    void loadSessionFromDatabase(int sessionId);
    QVector<TestSession> getHistoricalSessions(const QDateTime& from, const QDateTime& to, const QString& testType = "");

signals:
    void historicalDataLoaded(const QVector<DataPointRecord>& points);
    void sessionSaved(int sessionId);
    void sessionCreated(int sessionId);

private slots:
    void onDataPointsSaved(int count);
    void onDataPointsLoaded(const QVector<DataPointRecord>& points);
    void onTestSessionSaved(int sessionId);
    void autoSave(); // Автосохранение

private:
    mutable QReadWriteLock m_lock;
    QMap<QString, QVector<DataPoint>> m_data;
    DatabaseAsyncManager* m_dbManager;
    TestSession m_currentSession;
    bool m_sessionActive;
    QTimer* m_autoSaveTimer;

    void saveToDatabaseAsync();
};
