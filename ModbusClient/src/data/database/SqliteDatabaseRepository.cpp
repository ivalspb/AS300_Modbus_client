#include "SqliteDatabaseRepository.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

SqliteDatabaseRepository::SqliteDatabaseRepository(QObject* parent)
    : QObject(parent)
    , m_sessionDao(m_database)
    , m_dataPointDao(m_database)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_databasePath = dataDir + "/modbus_data.db";

    m_database = QSqlDatabase::addDatabase("QSQLITE", "modbus_connection");
    m_database.setDatabaseName(m_databasePath);
}

SqliteDatabaseRepository::SqliteDatabaseRepository(const QString& databasePath, QObject* parent)
    : QObject(parent)
    , m_sessionDao(m_database)
    , m_dataPointDao(m_database)
    , m_databasePath(databasePath)
{
    m_database = QSqlDatabase::addDatabase("QSQLITE", "modbus_connection");
    m_database.setDatabaseName(m_databasePath);
}

SqliteDatabaseRepository::~SqliteDatabaseRepository() {
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool SqliteDatabaseRepository::initializeDatabase() {
    if (!m_database.open()) {
        qCritical() << "Cannot open database:" << m_database.lastError().text();
        return false;
    }

    // Enable foreign keys
    QSqlQuery query(m_database);
    if (!query.exec("PRAGMA foreign_keys = ON")) {
        qWarning() << "Failed to enable foreign keys:" << query.lastError().text();
    }

    // Create tables
    if (!m_sessionDao.createTable()) {
        qCritical() << "Failed to create test_sessions table";
        return false;
    }

    if (!m_dataPointDao.createTable()) {
        qCritical() << "Failed to create data_points table";
        return false;
    }

    qDebug() << "Database initialized successfully:" << m_databasePath;
    return true;
}

int SqliteDatabaseRepository::createTestSession(const TestSession& session) {
    return m_sessionDao.insert(session);
}

bool SqliteDatabaseRepository::updateTestSession(const TestSession& session) {
    return m_sessionDao.update(session);
}

QVector<TestSession> SqliteDatabaseRepository::getTestSessions(const QDateTime& from,
                                                               const QDateTime& to,
                                                               const QString& testType) {
    return m_sessionDao.findSessions(from, to, testType);
}

bool SqliteDatabaseRepository::saveDataPoints(const QVector<DataPointRecord>& points) {
    return m_dataPointDao.insertBatch(points);
}

QVector<DataPointRecord> SqliteDatabaseRepository::getDataPoints(int sessionId,
                                                                 const QString& parameter) {
    return m_dataPointDao.findBySession(sessionId, parameter);
}

QVector<DataPointRecord> SqliteDatabaseRepository::getDataPointsByTimeRange(int sessionId,
                                                                            const QDateTime& from,
                                                                            const QDateTime& to,
                                                                            const QString& parameter) {
    return m_dataPointDao.findBySessionAndTimeRange(sessionId, from, to, parameter);
}

int SqliteDatabaseRepository::getSessionCount() {
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM test_sessions") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

qint64 SqliteDatabaseRepository::getTotalDataPoints() {
    QSqlQuery query(m_database);
    if (query.exec("SELECT COUNT(*) FROM data_points") && query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}
