#include "SpeedometerWidget.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

#ifndef M_PI
const double M_PI = 3.14159265358979323846;
#endif

SpeedometerWidget::SpeedometerWidget(const QString& title, double minValue, double maxValue, QWidget* parent)
    : QWidget(parent)
    , m_title(title)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_currentValue(minValue)
    , m_color(Qt::blue)
    , m_alertLevel(maxValue)
    , m_alertColor(Qt::red)
    , m_alertActive(false)
{
    setMinimumSize(200, 200);
}

void SpeedometerWidget::setValue(const double value) {
    m_currentValue = qBound(m_minValue, value, m_maxValue);
    m_alertActive = (m_currentValue >= m_alertLevel);
    update();
}

void SpeedometerWidget::setRange(const double min, const double max){
    m_minValue=min;
    m_maxValue=max;
    update();
}


void SpeedometerWidget::setColor(const QColor& color) {
    m_color = color;
    update();
}

void SpeedometerWidget::setAlertLevel(double level, const QColor& color) {
    m_alertLevel = level;
    m_alertColor = color;
    update();
}

void SpeedometerWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    painter.fillRect(rect(), Qt::white);

    // Рисуем компоненты спидометра
    drawBackground(painter);
    drawTicks(painter);
    drawLabels(painter);
    drawNeedle(painter);
    drawValue(painter);
}

void SpeedometerWidget::drawBackground(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);

    // Градиент для фона спидометра
    QConicalGradient gradient(rect.center(), -50);
    gradient.setColorAt(0, QColorConstants::Svg::lightsalmon);
    gradient.setColorAt(0.5, Qt::yellow);
    gradient.setColorAt(1, Qt::yellow);

    painter.setBrush(gradient);
    painter.setPen(QPen(Qt::black, 2));
    painter.drawEllipse(rect);
}

void SpeedometerWidget::drawTicks(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);
    QPointF center = rect.center();
    double radius = size / 3;

    painter.setPen(QPen(Qt::black, 2));

    // Основные деления
    for (int i = 0; i <= 11; ++i) {
        double angle = 225 - i * 270.0 / 11.0; // 225° до -45°
        double rad = angle * M_PI / 180.0;

        double innerRadius = radius * 0.7;
        double outerRadius = radius * 0.9;

        QPointF innerPoint(center.x() + innerRadius * cos(rad),
                           center.y() - innerRadius * sin(rad));
        QPointF outerPoint(center.x() + outerRadius * cos(rad),
                           center.y() - outerRadius * sin(rad));

        painter.drawLine(innerPoint, outerPoint);
    }

    // Мелкие деления
    painter.setPen(QPen(Qt::black, 1));
    for (int i = 0; i <= 55; ++i) {
        double angle = 225 - i * 270.0 / 55.0;
        double rad = angle * M_PI / 180.0;

        double innerRadius = radius * 0.8;
        double outerRadius = radius * 0.85;

        QPointF innerPoint(center.x() + innerRadius * cos(rad),
                           center.y() - innerRadius * sin(rad));
        QPointF outerPoint(center.x() + outerRadius * cos(rad),
                           center.y() - outerRadius * sin(rad));

        painter.drawLine(innerPoint, outerPoint);
    }
}

void SpeedometerWidget::drawLabels(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);
    QPointF center = rect.center();
    double radius = size / 2;

    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    for (int i = 0; i <= 11; ++i) {
        double value = m_minValue + i * (m_maxValue - m_minValue) / 11.0;
        double angle = 225 - i * 270.0 / 11.0;
        double rad = angle * M_PI / 180.0;

        double labelRadius = radius * 0.75;
        QPointF labelPoint(center.x() + labelRadius * cos(rad),
                           center.y() - labelRadius * sin(rad));

        QString label = QString::number(value, 'f', 0);
        QRectF textRect(labelPoint.x() - 20, labelPoint.y() - 10, 40, 20);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }

    // Заголовок
    font.setPointSize(12);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRectF(0, center.y() + radius * 0.4, width(), 30),
                     Qt::AlignCenter, m_title);
}

void SpeedometerWidget::drawNeedle(QPainter& painter) {
    int size = qMin(width(), height()) - 20;
    QRectF rect((width() - size) / 2, (height() - size) / 2, size, size);
    QPointF center = rect.center();
    double radius = size / 2;

    // Вычисляем угол стрелки
    double normalizedValue = (m_currentValue - m_minValue) / (m_maxValue - m_minValue);
    double angle = 225 - normalizedValue * 270.0; // 225° до -45°
    double rad = angle * M_PI / 180.0;

    // Рисуем стрелку
    QPen needlePen(m_alertActive ? m_alertColor : m_color, 4);
    painter.setPen(needlePen);

    double needleLength = radius * 0.8;
    QPointF needleEnd(center.x() + needleLength * cos(rad),
                      center.y() - needleLength * sin(rad));

    painter.drawLine(center, needleEnd);

    // Центральная точка
    painter.setBrush(m_alertActive ? m_alertColor : m_color);
    painter.drawEllipse(center, 5, 5);
}

void SpeedometerWidget::drawValue(QPainter& painter) {
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    painter.setPen(m_alertActive ? m_alertColor : m_color);

    QString valueText = QString("%1").arg(m_currentValue, 0, 'f', 0);
    QRectF valueRect(0, height() * 0.6, width(), 10);
    painter.drawText(valueRect, Qt::AlignCenter, valueText);
}
