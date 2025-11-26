#pragma once
#include "IDatabaseRepository.h"
#include "TestSession.h"
#include <QObject>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QVariant>
#include <QMutex>
#include <QWaitCondition>

// Регистрация метатипов для сигналов/слотов
Q_DECLARE_METATYPE(QVector<TestSession>)
Q_DECLARE_METATYPE(QVector<DataPointRecord>)

class DatabaseAsyncManager : public QObject {
    Q_OBJECT
public:
    explicit DatabaseAsyncManager(IDatabaseRepository* repository, QObject* parent = nullptr);
    ~DatabaseAsyncManager() override;

    void start();
    void stop();

    void saveTestSession(const TestSession& session);
    void saveDataPoints(const QVector<DataPointRecord>& points);
    void loadTestSessions(const QDateTime& from, const QDateTime& to, const QString& testType = "");
    void loadDataPoints(int sessionId, const QString& parameter = "");

signals:
    void testSessionSaved(int sessionId);
    void dataPointsSaved(int count);
    void testSessionsLoaded(const QVector<TestSession>& sessions);
    void dataPointsLoaded(const QVector<DataPointRecord>& points);
    void errorOccurred(const QString& error);

private slots:
    void processQueue();

private:
    enum OperationType {
        SaveSession,
        SaveDataPoints,
        LoadSessions,
        LoadDataPoints
    };

    struct DatabaseOperation {
        OperationType type;
        QVariant data;
        QVariant extra;
    };

    IDatabaseRepository* m_repository;
    QThread* m_workerThread;
    QQueue<DatabaseOperation> m_operationQueue;
    QMutex m_queueMutex;
    QWaitCondition m_queueCondition;
    bool m_running;

    void addOperation(const DatabaseOperation& operation);
};
