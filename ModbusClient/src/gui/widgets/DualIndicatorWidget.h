#pragma once
#include <QWidget>
#include "SpeedometerWidget.h"
#include "core/interfaces/IIndicator.h"

class QLabel;
class QVBoxLayout;

/**
 * @brief Композитный виджет, объединяющий спидометр и LCD индикатор
 * Реализует принцип SRP - отвечает только за компоновку двух индикаторов
 */
class DualIndicatorWidget : public QWidget, public IDualIndicator {
    Q_OBJECT
public:
    explicit DualIndicatorWidget(const QString& primaryTitle,
                               const QString& secondaryTitle,
                               double primaryMin, double primaryMax,
                               double secondaryMin, double secondaryMax,
                               QWidget* parent = nullptr);

    // Методы для управления спидометром
    void setValue(double value);
    void setRange(double min, double max);
    void setColor(const QColor& color);
    void setAlertLevel(double level, const QColor& color = Qt::red);

    // Методы для управления LCD индикатором
    void setSecondaryValue(double value);
    void setSecondaryRange(double min, double max);
    void setSecondaryVisible(bool visible);

    // Получение внутренних компонентов (для обратной совместимости)
    SpeedometerWidget* speedometer() const { return m_speedometer; }
    QLabel* lcdLabel() const { return m_lcdLabel; }

private:
    void updateLcdDisplay();

    SpeedometerWidget* m_speedometer;
    QLabel* m_lcdLabel;
    QString m_secondaryTitle;
    double m_secondaryValue;
    double m_secondaryMin;
    double m_secondaryMax;
};
