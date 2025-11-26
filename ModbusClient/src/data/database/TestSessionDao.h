#pragma once
#include "TestSession.h"
#include <QSqlDatabase>
#include <QVector>

class TestSessionDao {
public:
    explicit TestSessionDao(QSqlDatabase& database);

    bool createTable();
    int insert(const TestSession& session);
    bool update(const TestSession& session);
    QVector<TestSession> findSessions(const QDateTime& from,
                                      const QDateTime& to,
                                      const QString& testType = "");
    TestSession findById(int id);

private:
    QSqlDatabase& m_database;
};
