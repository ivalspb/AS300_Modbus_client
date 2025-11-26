#pragma once
#include <QDateTime>
#include <QString>

struct DataPoint {
    QDateTime timestamp;
    QString parameter;
    double value;

    DataPoint() : value(0.0) {}

    DataPoint(const QString& param, double val)
        : timestamp(QDateTime::currentDateTime())
        , parameter(param)
        , value(val)
    {}
};
