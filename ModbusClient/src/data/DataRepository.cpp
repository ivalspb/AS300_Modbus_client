
#include "DataRepository.h"
#include <QWriteLocker>
#include <QReadLocker>
#include <QDebug>

DataRepository::DataRepository(DatabaseAsyncManager* dbManager, QObject* parent)
    : IDataRepository(parent)
    , m_dbManager(dbManager)
    , m_sessionActive(false)
    , m_autoSaveTimer(new QTimer(this))
{
    if (m_dbManager) {
        connect(m_dbManager, SIGNAL(dataPointsSaved(int)),
                this, SLOT(onDataPointsSaved(int)));
        connect(m_dbManager, SIGNAL(dataPointsLoaded(QVector<DataPointRecord>)),
                this, SLOT(onDataPointsLoaded(QVector<DataPointRecord>)));
    }

    // Автосохранение каждые 30 секунд
    m_autoSaveTimer->setInterval(30000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &DataRepository::autoSave);

    qDebug() << "DataRepository: Initialized with auto-save every 30 seconds";
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

    // Сразу создаём сессию в БД
    if (m_dbManager) {
        // Сохраняем сессию и получаем ID через сигнал
        connect(m_dbManager, &DatabaseAsyncManager::testSessionSaved,
                this, &DataRepository::onTestSessionSaved, Qt::UniqueConnection);
        m_dbManager->saveTestSession(m_currentSession);
        qDebug() << "DataRepository: Creating new test session in database:" << testType;
    }

    // Запускаем автосохранение
    if (m_autoSaveTimer && !m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->start();
        qDebug() << "DataRepository: Auto-save timer started";
    }
}

void DataRepository::onTestSessionSaved(int sessionId) {
    if (sessionId > 0) {
        m_currentSession.id = sessionId;
        qDebug() << "DataRepository: Test session saved with ID:" << sessionId;
        disconnect(m_dbManager, &DatabaseAsyncManager::testSessionSaved,
                   this, &DataRepository::onTestSessionSaved);

        emit sessionCreated(sessionId);
    } else {
        qCritical() << "DataRepository: Failed to get valid session ID";
    }
}

void DataRepository::saveCurrentSessionToDatabase() {
    if (!m_dbManager || !m_sessionActive) {
        qDebug() << "DataRepository: Cannot save - no active session or no database manager";
        return;
    }

    // Останавливаем автосохранение
    if (m_autoSaveTimer && m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->stop();
        qDebug() << "DataRepository: Auto-save timer stopped";
    }

    m_currentSession.endTime = QDateTime::currentDateTime();

    // Обновляем сессию в БД
    m_dbManager->saveTestSession(m_currentSession);

    // Сохраняем все накопленные данные
    saveToDatabaseAsync();

    qDebug() << "DataRepository: Test session saved to database. Duration:"
             << m_currentSession.startTime.secsTo(m_currentSession.endTime) << "seconds";
}

void DataRepository::finalizeSession() {
    if (m_sessionActive) {
        qDebug() << "DataRepository: Finalizing active session (emergency save)";
        saveCurrentSessionToDatabase();
        m_sessionActive = false;
    }
}

void DataRepository::saveToDatabaseAsync() {
    if (!m_dbManager || m_data.isEmpty() || m_currentSession.id <= 0) {
        qDebug() << "DataRepository: Nothing to save or no valid session ID";
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
        qDebug() << "DataRepository: Saving" << points.size() << "data points to database";
    }
}

void DataRepository::loadSessionFromDatabase(int sessionId) {
    if (m_dbManager) {
        // Загружаем все параметры для сессии
        m_dbManager->loadDataPoints(sessionId);
        qDebug() << "DataRepository: Loading session" << sessionId << "from database";
    }
}

QVector<TestSession> DataRepository::getHistoricalSessions(const QDateTime& from, const QDateTime& to, const QString& testType) {
    // Этот метод должен быть реализован через DatabaseAsyncManager
    // Пока возвращаем пустой список
    return QVector<TestSession>();
}

void DataRepository::onDataPointsSaved(int count) {
    qDebug() << "DataRepository: Successfully saved" << count << "data points to database";
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
    qDebug() << "DataRepository: Loaded" << points.size() << "historical data points";
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
        qDebug() << "DataRepository: All data cleared";
    } else {
        m_data.remove(parameter);
        qDebug() << "DataRepository: Data cleared for parameter:" << parameter;
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

void DataRepository::autoSave() {
    if (!m_sessionActive || !m_dbManager) {
        return;
    }

    // Периодически сохраняем накопленные данные (не завершая сессию)
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
        qDebug() << "DataRepository: Auto-save triggered -" << points.size() << "points saved";
    }
}
