#pragma once
#include "IDatabaseRepository.h"
#include "TestSessionDao.h"
#include "DataPointDao.h"
#include <QSqlDatabase>
#include <QString>
#include <QObject>

class SqliteDatabaseRepository : public QObject, public IDatabaseRepository {
    Q_OBJECT
public:
    explicit SqliteDatabaseRepository(QObject* parent = nullptr);
    explicit SqliteDatabaseRepository(const QString& databasePath, QObject* parent = nullptr);
    ~SqliteDatabaseRepository() override;

    // IDatabaseRepository interface
    bool initializeDatabase() override;
    int createTestSession(const TestSession& session) override;
    bool updateTestSession(const TestSession& session) override;
    QVector<TestSession> getTestSessions(const QDateTime& from,
                                        const QDateTime& to,
                                        const QString& testType = "") override;

    bool saveDataPoints(const QVector<DataPointRecord>& points) override;
    QVector<DataPointRecord> getDataPoints(int sessionId,
                                          const QString& parameter = "") override;
    QVector<DataPointRecord> getDataPointsByTimeRange(int sessionId,
                                                     const QDateTime& from,
                                                     const QDateTime& to,
                                                     const QString& parameter = "") override;

    int getSessionCount() override;
    qint64 getTotalDataPoints() override;

private:
    QSqlDatabase m_database;
    TestSessionDao m_sessionDao;
    DataPointDao m_dataPointDao;
    QString m_databasePath;
};
