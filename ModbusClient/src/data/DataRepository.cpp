#include "DataRepository.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <QDebug>

DataRepository::DataRepository(DatabaseAsyncManager* dbManager, QObject* parent)
    : IDataRepository(parent)
    , m_dbManager(dbManager)
    , m_sessionActive(false)
{
    if (m_dbManager) {
        connect(m_dbManager, SIGNAL(dataPointsSaved(int)),
                        this, SLOT(onDataPointsSaved(int)));
        connect(m_dbManager, SIGNAL(dataPointsLoaded(QVector<DataPointRecord>)),
                        this, SLOT(onDataPointsLoaded(QVector<DataPointRecord>)));
    }
}

void DataRepository::addDataPoint(const QString& parameter, double value) {
    QWriteLocker locker(&m_lock);

    DataPoint point(parameter, value);
    m_data[parameter].append(point);

    locker.unlock();
    emit dataAdded(parameter, value);
}

void DataRepository::setCurrentTestSession(const QString& testType) {
    m_currentSession = TestSession(testType, QDateTime::currentDateTime());
    m_sessionActive = true;

    // Очищаем данные для новой сессии
    clearData();
}

void DataRepository::saveCurrentSessionToDatabase() {
    if (!m_dbManager || !m_sessionActive) {
        return;
    }

    m_currentSession.endTime = QDateTime::currentDateTime();
    m_dbManager->saveTestSession(m_currentSession);

    // Данные будут сохранены асинхронно в onTestSessionSaved
}

void DataRepository::saveToDatabaseAsync() {
    if (!m_dbManager || m_data.isEmpty()) {
        return;
    }

    QVector<DataPointRecord> points;
    QReadLocker locker(&m_lock);

    for (auto it = m_data.constBegin(); it != m_data.constEnd(); ++it) {
        for (const auto& point : it.value()) {
            points.append(DataPointRecord(m_currentSession.id, it.key(), point.value, point.timestamp));
        }
    }

    locker.unlock();

    if (!points.isEmpty()) {
        m_dbManager->saveDataPoints(points);
    }
}

void DataRepository::loadSessionFromDatabase(int sessionId) {
    if (m_dbManager) {
        // Загружаем все параметры для сессии
        m_dbManager->loadDataPoints(sessionId);
    }
}

QVector<TestSession> DataRepository::getHistoricalSessions(const QDateTime& from, const QDateTime& to, const QString& testType) {
    // Этот метод должен быть реализован через DatabaseAsyncManager
    // Пока возвращаем пустой список
    return QVector<TestSession>();
}

void DataRepository::onDataPointsSaved(int count) {
    qDebug() << "Successfully saved" << count << "data points to database";
    emit sessionSaved(m_currentSession.id);
}

void DataRepository::onDataPointsLoaded(const QVector<DataPointRecord>& points) {
    // Очищаем текущие данные
    clearData();

    // Загружаем исторические данные
    QWriteLocker locker(&m_lock);
    for (const auto& point : points) {
        DataPoint dataPoint(point.parameter, point.value);
        dataPoint.timestamp = point.timestamp;
        m_data[point.parameter].append(dataPoint);
    }
    locker.unlock();

    emit historicalDataLoaded(points);
}

QVector<DataPoint> DataRepository::getDataPoints(const QString& parameter,
                                                 const QDateTime& from,
                                                 const QDateTime& to) const {
    QReadLocker locker(&m_lock);

    if (!m_data.contains(parameter)) {
        return QVector<DataPoint>();
    }

    const auto& points = m_data[parameter];
    QVector<DataPoint> result;
    result.reserve(points.size());

    for (const auto& point : points) {
        if ((from.isNull() || point.timestamp >= from) &&
            (to.isNull() || point.timestamp <= to)) {
            result.append(point);
        }
    }

    return result;
}

QVector<QString> DataRepository::getAvailableParameters() const {
    QReadLocker locker(&m_lock);
    return m_data.keys().toVector();
}

void DataRepository::clearData(const QString& parameter) {
    QWriteLocker locker(&m_lock);

    if (parameter.isEmpty()) {
        m_data.clear();
    } else {
        m_data.remove(parameter);
    }

    locker.unlock();
    emit dataCleared(parameter);
}

int DataRepository::getDataPointCount(const QString& parameter) const {
    QReadLocker locker(&m_lock);

    if (parameter.isEmpty()) {
        int total = 0;
        for (const auto& vec : m_data) {
            total += vec.size();
        }
        return total;
    }

    return m_data.value(parameter).size();
}

