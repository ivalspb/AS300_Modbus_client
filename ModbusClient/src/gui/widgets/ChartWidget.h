#pragma once
#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>
#include <QSplitter>
#include "SpeedometerWidget.h"
#include "data/database/TestSession.h"

QT_CHARTS_USE_NAMESPACE

    class IDataRepository;

class ChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChartWidget(QWidget* parent = nullptr);

    void setDataRepository(IDataRepository* repository);
    void setParameter(const QString& parameter);
    void updateChart();
    void setTimeRange(int seconds);
    void setYRange(double min, double max);

    void startTestRecording();
    void stopTestRecording();
    void clearChart();
    void loadHistoricalData(const QVector<DataPointRecord>& points);
    bool isRecording() const { return m_recording; }

private slots:
    void onAutoScaleADChanged(int state);
    void onAutoScaleTKChanged(int state);
    void onAutoScaleSTChanged(int state);
    void onManualScaleADChanged();
    void onManualScaleTKChanged();
    void onManualScaleSTChanged();
    void onTimeRangeChanged();
    void onGridToggled(bool enabled);
    void resetZoom();

private:
    void setupChart();
    void setupMultiAxis();
    void setupControlPanel();
    void setupControlPanelValues();
    void applyManualScale();
    void updateGrid();

    IDataRepository* m_repository;
    QString m_parameter;
    int m_timeRange;
    bool m_recording;

    // QSplitter* m_mainSplitter;

    // Левая панель - спидометры
    // QWidget* m_speedometerPanel;
    // SpeedometerWidget* m_speedometerAD;
    // SpeedometerWidget* m_speedometerTK;
    // SpeedometerWidget* m_speedometerST;

    // Правая панель - графики
    QWidget* m_chartPanel;
    QChart* m_chart;
    QChartView* m_chartView;
    QLineSeries* m_seriesAD;
    QLineSeries* m_seriesTK;
    QLineSeries* m_seriesST;
    QDateTimeAxis* m_axisX;
    QValueAxis* m_axisY_AD;
    QValueAxis* m_axisY_TK;
    QValueAxis* m_axisY_ST;

    // Элементы управления
    QCheckBox* m_autoScaleAD;
    QCheckBox* m_autoScaleTK;
    QCheckBox* m_autoScaleST;
    QDoubleSpinBox* m_minAD;
    QDoubleSpinBox* m_maxAD;
    QDoubleSpinBox* m_minTK;
    QDoubleSpinBox* m_maxTK;
    QDoubleSpinBox* m_minST;
    QDoubleSpinBox* m_maxST;
    QSpinBox* m_timeRangeSpin;
    QCheckBox* m_showGrid;
    QPushButton* m_resetZoomButton;
};
