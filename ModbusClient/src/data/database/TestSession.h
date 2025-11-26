#pragma once
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QMetaType>

struct TestSession {
    int id;
    QString testType;
    QDateTime startTime;
    QDateTime endTime;
    QString description;

    TestSession() : id(-1) {}
    TestSession(const QString& type, const QDateTime& start)
        : id(-1), testType(type), startTime(start) {}
};

Q_DECLARE_METATYPE(TestSession)

struct DataPointRecord {
    int id;
    int sessionId;
    QString parameter;
    double value;
    QDateTime timestamp;

    DataPointRecord() : id(-1), sessionId(-1), value(0.0) {}
    DataPointRecord(int sessId, const QString& param, double val, const QDateTime& time)
        : id(-1), sessionId(sessId), parameter(param), value(val), timestamp(time) {}
};
