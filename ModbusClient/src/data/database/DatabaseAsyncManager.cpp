#include "DatabaseAsyncManager.h"
#include <QMetaType>
#include <QDebug>

DatabaseAsyncManager::DatabaseAsyncManager(IDatabaseRepository* repository, QObject* parent)
    : QObject(parent)
    , m_repository(repository)
    , m_workerThread(new QThread(this))
    , m_running(false)
{
    qRegisterMetaType<QVector<TestSession>>("QVector<TestSession>");
    qRegisterMetaType<QVector<DataPointRecord>>("QVector<DataPointRecord>");

    moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::started, this, &DatabaseAsyncManager::processQueue);
}

DatabaseAsyncManager::~DatabaseAsyncManager() {
    stop();
}

void DatabaseAsyncManager::start() {
    if (!m_running) {
        m_running = true;
        m_workerThread->start();
        qDebug() << "Database async manager started";
    }
}

void DatabaseAsyncManager::stop() {
    if (m_running) {
        m_running = false;
        m_queueCondition.wakeAll();
        m_workerThread->quit();
        m_workerThread->wait(5000);
        qDebug() << "Database async manager stopped";
    }
}

void DatabaseAsyncManager::saveTestSession(const TestSession& session) {
    DatabaseOperation op;
    op.type = SaveSession;
    op.data.setValue(session);
    addOperation(op);
}

void DatabaseAsyncManager::saveDataPoints(const QVector<DataPointRecord>& points) {
    DatabaseOperation op;
    op.type = SaveDataPoints;
    op.data.setValue(points);
    addOperation(op);
}

void DatabaseAsyncManager::loadTestSessions(const QDateTime& from, const QDateTime& to, const QString& testType) {
    DatabaseOperation op;
    op.type = LoadSessions;
    op.data.setValue(QPair<QDateTime, QDateTime>(from, to));
    op.extra.setValue(testType);
    addOperation(op);
}

void DatabaseAsyncManager::loadDataPoints(int sessionId, const QString& parameter) {
    DatabaseOperation op;
    op.type = LoadDataPoints;
    op.data.setValue(sessionId);
    op.extra.setValue(parameter);
    addOperation(op);
}

void DatabaseAsyncManager::addOperation(const DatabaseOperation& operation) {
    QMutexLocker locker(&m_queueMutex);
    m_operationQueue.enqueue(operation);
    m_queueCondition.wakeOne();
}

void DatabaseAsyncManager::processQueue() {
    while (m_running) {
        DatabaseOperation operation;

        {
            QMutexLocker locker(&m_queueMutex);
            if (m_operationQueue.isEmpty()) {
                m_queueCondition.wait(&m_queueMutex);
                if (!m_running) break;
            }
            operation = m_operationQueue.dequeue();
        }

        try {
            switch (operation.type) {
            case SaveSession: {
                TestSession session = operation.data.value<TestSession>();
                int sessionId = m_repository->createTestSession(session);
                if (sessionId > 0) {
                    emit testSessionSaved(sessionId);
                } else {
                    emit errorOccurred("Failed to save test session");
                }
                break;
            }
            case SaveDataPoints: {
                QVector<DataPointRecord> points = operation.data.value<QVector<DataPointRecord>>();
                bool success = m_repository->saveDataPoints(points);
                if (success) {
                    emit dataPointsSaved(points.size());
                } else {
                    emit errorOccurred("Failed to save data points");
                }
                break;
            }
            case LoadSessions: {
                auto range = operation.data.value<QPair<QDateTime, QDateTime>>();
                QString testType = operation.extra.toString();
                QVector<TestSession> sessions = m_repository->getTestSessions(range.first, range.second, testType);
                emit testSessionsLoaded(sessions);
                break;
            }
            case LoadDataPoints: {
                int sessionId = operation.data.toInt();
                QString parameter = operation.extra.toString();
                QVector<DataPointRecord> points = m_repository->getDataPoints(sessionId, parameter);
                emit dataPointsLoaded(points);
                break;
            }
            }
        } catch (const std::exception& e) {
            emit errorOccurred(QString("Database operation failed: %1").arg(e.what()));
        }
    }
}
