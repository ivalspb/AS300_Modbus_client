#include "DualIndicatorWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>

DualIndicatorWidget::DualIndicatorWidget(const QString& primaryTitle,
                                       const QString& secondaryTitle,
                                       double primaryMin, double primaryMax,
                                       double secondaryMin, double secondaryMax,
                                       QWidget* parent)
    : QWidget(parent)
    , m_speedometer(new SpeedometerWidget(primaryTitle, primaryMin, primaryMax, this))
    , m_lcdLabel(new QLabel(this))
    , m_secondaryTitle(secondaryTitle)
    , m_secondaryValue(0.0)
    , m_secondaryMin(secondaryMin)
    , m_secondaryMax(secondaryMax)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(2, 2, 2, 2);

    layout->addWidget(m_speedometer);
    layout->addWidget(m_lcdLabel);

    // Настройка LCD стиля
    m_lcdLabel->setAlignment(Qt::AlignCenter);
    m_lcdLabel->setMinimumHeight(25);
    m_lcdLabel->setStyleSheet(
        "QLabel {"
        "background-color: #f0f0f0;"
        "border: 2px solid #c0c0c0;"
        "border-radius: 3px;"
        "padding: 2px 4px;"
        "font-weight: bold;"
        "font-family: 'Courier New', monospace;"
        "color: #000000;"
        "}"
    );

    // Устанавливаем шрифт фиксированной ширины для LCD эффекта
    QFont lcdFont("Courier New", 10, QFont::Bold);
    m_lcdLabel->setFont(lcdFont);

    updateLcdDisplay();
}

void DualIndicatorWidget::setValue(double value) {
    m_speedometer->setValue(value);
}

void DualIndicatorWidget::setRange(double min, double max) {
    m_speedometer->setRange(min, max);
}

void DualIndicatorWidget::setColor(const QColor& color) {
    m_speedometer->setColor(color);
}

void DualIndicatorWidget::setAlertLevel(double level, const QColor& color) {
    m_speedometer->setAlertLevel(level, color);
}

void DualIndicatorWidget::setSecondaryValue(double value) {
    m_secondaryValue = qBound(m_secondaryMin, value, m_secondaryMax);
    updateLcdDisplay();
}

void DualIndicatorWidget::setSecondaryRange(double min, double max) {
    m_secondaryMin = min;
    m_secondaryMax = max;
    // Обновляем текущее значение в рамках нового диапазона
    m_secondaryValue = qBound(min, m_secondaryValue, max);
    updateLcdDisplay();
}

void DualIndicatorWidget::setSecondaryVisible(bool visible) {
    m_lcdLabel->setVisible(visible);
}

void DualIndicatorWidget::updateLcdDisplay() {
    QString valueText = QString("%1").arg(m_secondaryValue, 0, 'f', 0);
    QString text = QString("%1\n%2")
                   .arg(valueText)
                   .arg(m_secondaryTitle);
    m_lcdLabel->setText(text);

    // Опционально: меняем цвет если значение превышает 100%
    if (m_speedometer && m_speedometer->value() > 100.0) {
        m_lcdLabel->setStyleSheet(
            "QLabel {"
            "background-color: #fff0f0;"
            "border: 2px solid #ff0000;"
            "border-radius: 3px;"
            "padding: 2px 4px;"
            "font-weight: bold;"
            "font-family: 'Courier New', monospace;"
            "color: #ff0000;"
            "}"
        );
    } else {
        m_lcdLabel->setStyleSheet(
            "QLabel {"
            "background-color: #f0f0f0;"
            "border: 2px solid #c0c0c0;"
            "border-radius: 3px;"
            "padding: 2px 4px;"
            "font-weight: bold;"
            "font-family: 'Courier New', monospace;"
            "color: #000000;"
            "}"
        );
    }
}
