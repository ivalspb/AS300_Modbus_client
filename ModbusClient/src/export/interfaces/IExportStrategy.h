#pragma once
#include <QWidget>
#include <QString>

class IExportStrategy {
public:
    virtual ~IExportStrategy() = default;
    virtual bool exportWidget(QWidget* widget, const QString& filename) = 0;
};
