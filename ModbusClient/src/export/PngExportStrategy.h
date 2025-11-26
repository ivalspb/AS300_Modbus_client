#pragma once
#include "interfaces/IExportStrategy.h"

class PngExportStrategy : public IExportStrategy {
public:
    bool exportWidget(QWidget* widget, const QString& filename) override;
};
