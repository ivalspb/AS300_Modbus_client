#pragma once
#include <QObject>
#include <QVector>
#include <QDateTime>
#include "../DataPoint.h"

class IDataRepository : public QObject {
    Q_OBJECT
public:
    explicit IDataRepository(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IDataRepository() = default;

    virtual void addDataPoint(const QString& parameter, double value) = 0;
    virtual QVector<DataPoint> getDataPoints(const QString& parameter,
                                             const QDateTime& from = QDateTime(),
                                             const QDateTime& to = QDateTime()) const = 0;
    virtual QVector<QString> getAvailableParameters() const = 0;
    virtual void clearData(const QString& parameter = QString()) = 0;
    virtual int getDataPointCount(const QString& parameter) const = 0;

signals:
    void dataAdded(const QString& parameter, double value);
    void dataCleared(const QString& parameter);
};
// const QString& parameter);
// };
