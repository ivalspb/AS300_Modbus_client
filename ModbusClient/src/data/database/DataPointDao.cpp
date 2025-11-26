#include "DataPointDao.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DataPointDao::DataPointDao(QSqlDatabase& database)
    : m_database(database)
{}

bool DataPointDao::createTable() {
    QSqlQuery query(m_database);
    return query.exec(
        "CREATE TABLE IF NOT EXISTS data_points ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "session_id INTEGER NOT NULL, "
        "parameter TEXT NOT NULL, "
        "value REAL NOT NULL, "
        "timestamp DATETIME NOT NULL, "
        "FOREIGN KEY(session_id) REFERENCES test_sessions(id) ON DELETE CASCADE"
        ")"
        );
}

bool DataPointDao::insertBatch(const QVector<DataPointRecord>& points) {
    if (points.isEmpty()) return true;

    m_database.transaction();

    QSqlQuery query(m_database);
    query.prepare(
        "INSERT INTO data_points (session_id, parameter, value, timestamp) "
        "VALUES (:session_id, :parameter, :value, :timestamp)"
        );

    for (const auto& point : points) {
        query.bindValue(":session_id", point.sessionId);
        query.bindValue(":parameter", point.parameter);
        query.bindValue(":value", point.value);
        query.bindValue(":timestamp", point.timestamp);

        if (!query.exec()) {
            m_database.rollback();
            qWarning() << "Failed to insert data point:" << query.lastError().text();
            return false;
        }
    }

    return m_database.commit();
}

QVector<DataPointRecord> DataPointDao::findBySession(int sessionId, const QString& parameter) {
    QVector<DataPointRecord> points;
    QSqlQuery query(m_database);

    QString sql = "SELECT id, session_id, parameter, value, timestamp "
                  "FROM data_points WHERE session_id = :session_id";

    if (!parameter.isEmpty()) {
        sql += " AND parameter = :parameter";
    }
    sql += " ORDER BY timestamp ASC";

    query.prepare(sql);
    query.bindValue(":session_id", sessionId);
    if (!parameter.isEmpty()) {
        query.bindValue(":parameter", parameter);
    }

    if (query.exec()) {
        while (query.next()) {
            DataPointRecord point;
            point.id = query.value("id").toInt();
            point.sessionId = query.value("session_id").toInt();
            point.parameter = query.value("parameter").toString();
            point.value = query.value("value").toDouble();
            point.timestamp = query.value("timestamp").toDateTime();
            points.append(point);
        }
    }

    return points;
}

QVector<DataPointRecord> DataPointDao::findBySessionAndTimeRange(int sessionId,
                                                                 const QDateTime& from,
                                                                 const QDateTime& to,
                                                                 const QString& parameter) {
    QVector<DataPointRecord> points;
    QSqlQuery query(m_database);

    QString sql = "SELECT id, session_id, parameter, value, timestamp "
                  "FROM data_points WHERE session_id = :session_id "
                  "AND timestamp BETWEEN :from AND :to";

    if (!parameter.isEmpty()) {
        sql += " AND parameter = :parameter";
    }
    sql += " ORDER BY timestamp ASC";

    query.prepare(sql);
    query.bindValue(":session_id", sessionId);
    query.bindValue(":from", from);
    query.bindValue(":to", to);
    if (!parameter.isEmpty()) {
        query.bindValue(":parameter", parameter);
    }

    if (query.exec()) {
        while (query.next()) {
            DataPointRecord point;
            point.id = query.value("id").toInt();
            point.sessionId = query.value("session_id").toInt();
            point.parameter = query.value("parameter").toString();
            point.value = query.value("value").toDouble();
            point.timestamp = query.value("timestamp").toDateTime();
            points.append(point);
        }
    }

    return points;
}

int DataPointDao::getPointCount(int sessionId) {
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM data_points WHERE session_id = :session_id");
    query.bindValue(":session_id", sessionId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
