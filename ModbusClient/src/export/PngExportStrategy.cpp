#include "PngExportStrategy.h"
#include <QPainter>
#include <QPixmap>

bool PngExportStrategy::exportWidget(QWidget* widget, const QString& filename) {
    if (!widget) {
        return false;
    }

    QPixmap pixmap(widget->size());
    widget->render(&pixmap);

    return pixmap.save(filename, "PNG");
}
