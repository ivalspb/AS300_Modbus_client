#pragma once
#include <QWidget>
#include <QPainter>
#include <QConicalGradient>
#include <QTimer>

class SpeedometerWidget : public QWidget {
    Q_OBJECT
public:
    explicit SpeedometerWidget(const QString& title, double minValue, double maxValue,
                               QWidget* parent = nullptr);

    void setValue(const double value);
    inline double value() const {return m_currentValue;}
    void setRange(const double min, const double max);
    void setColor(const QColor& color);
    void setAlertLevel(double level, const QColor& color = Qt::red);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void drawBackground(QPainter& painter);
    void drawTicks(QPainter& painter);
    void drawLabels(QPainter& painter);
    void drawNeedle(QPainter& painter);
    void drawValue(QPainter& painter);

    QString m_title;
    double m_minValue;
    double m_maxValue;
    double m_currentValue;
    QColor m_color;
    double m_alertLevel;
    QColor m_alertColor;
    bool m_alertActive;
};
