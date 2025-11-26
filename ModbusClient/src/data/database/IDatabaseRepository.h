#pragma once
#include "TestSession.h"
#include <QVector>
#include <QDateTime>

class IDatabaseRepository {
public:
    virtual ~IDatabaseRepository() = default;

    // Session management
    virtual bool initializeDatabase() = 0;
    virtual int createTestSession(const TestSession& session) = 0;
    virtual bool updateTestSession(const TestSession& session) = 0;
    virtual QVector<TestSession> getTestSessions(const QDateTime& from,
                                                 const QDateTime& to,
                                                 const QString& testType = "") = 0;

    // Data points
    virtual bool saveDataPoints(const QVector<DataPointRecord>& points) = 0;
    virtual QVector<DataPointRecord> getDataPoints(int sessionId,
                                                   const QString& parameter = "") = 0;
    virtual QVector<DataPointRecord> getDataPointsByTimeRange(int sessionId,
                                                              const QDateTime& from,
                                                              const QDateTime& to,
                                                              const QString& parameter = "") = 0;

    // Statistics
    virtual int getSessionCount() = 0;
    virtual qint64 getTotalDataPoints() = 0;
};
