#include "TestSessionDao.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

TestSessionDao::TestSessionDao(QSqlDatabase& database)
    : m_database(database)
{}

bool TestSessionDao::createTable() {
    QSqlQuery query(m_database);
    return query.exec(
        "CREATE TABLE IF NOT EXISTS test_sessions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "test_type TEXT NOT NULL, "
        "start_time DATETIME NOT NULL, "
        "end_time DATETIME, "
        "description TEXT"
        ")"
        );
}

int TestSessionDao::insert(const TestSession& session) {
    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO test_sessions (test_type, start_time, end_time, description) "
        "VALUES (:test_type, :start_time, :end_time, :description)"
        );
    query.bindValue(":test_type", session.testType);
    query.bindValue(":start_time", session.startTime);
    query.bindValue(":end_time", session.endTime);
    query.bindValue(":description", session.description);

    if (query.exec()) {
        return query.lastInsertId().toInt();
    } else {
        qWarning() << "Failed to insert test session:" << query.lastError().text();
        return -1;
    }
}

bool TestSessionDao::update(const TestSession& session) {
    QSqlQuery query(m_database);
    query.prepare(
        "UPDATE test_sessions SET "
        "test_type = :test_type, start_time = :start_time, "
        "end_time = :end_time, description = :description "
        "WHERE id = :id"
        );
    query.bindValue(":test_type", session.testType);
    query.bindValue(":start_time", session.startTime);
    query.bindValue(":end_time", session.endTime);
    query.bindValue(":description", session.description);
    query.bindValue(":id", session.id);

    return query.exec();
}

QVector<TestSession> TestSessionDao::findSessions(const QDateTime& from,
                                                  const QDateTime& to,
                                                  const QString& testType) {
    QVector<TestSession> sessions;
    QSqlQuery query(m_database);

    QString sql = "SELECT id, test_type, start_time, end_time, description "
                  "FROM test_sessions WHERE start_time BETWEEN :from AND :to";

    if (!testType.isEmpty()) {
        sql += " AND test_type = :test_type";
    }
    sql += " ORDER BY start_time DESC";

    query.prepare(sql);
    query.bindValue(":from", from);
    query.bindValue(":to", to);
    if (!testType.isEmpty()) {
        query.bindValue(":test_type", testType);
    }

    if (query.exec()) {
        while (query.next()) {
            TestSession session;
            session.id = query.value("id").toInt();
            session.testType = query.value("test_type").toString();
            session.startTime = query.value("start_time").toDateTime();
            session.endTime = query.value("end_time").toDateTime();
            session.description = query.value("description").toString();
            sessions.append(session);
        }
    }

    return sessions;
}

TestSession TestSessionDao::findById(int id) {
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM test_sessions WHERE id = :id");
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        TestSession session;
        session.id = query.value("id").toInt();
        session.testType = query.value("test_type").toString();
        session.startTime = query.value("start_time").toDateTime();
        session.endTime = query.value("end_time").toDateTime();
        session.description = query.value("description").toString();
        return session;
    }

    return TestSession();
}
