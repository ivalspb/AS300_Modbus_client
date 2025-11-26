#pragma once
#include "TestSession.h"
#include <QSqlDatabase>
#include <QVector>

class DataPointDao {
public:
    explicit DataPointDao(QSqlDatabase& database);

    bool createTable();
    bool insertBatch(const QVector<DataPointRecord>& points);
    QVector<DataPointRecord> findBySession(int sessionId, const QString& parameter = "");
    QVector<DataPointRecord> findBySessionAndTimeRange(int sessionId,
                                                       const QDateTime& from,
                                                       const QDateTime& to,
                                                       const QString& parameter = "");
    int getPointCount(int sessionId);

private:
    QSqlDatabase& m_database;
};
