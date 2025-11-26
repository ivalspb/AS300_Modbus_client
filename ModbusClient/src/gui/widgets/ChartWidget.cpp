#include "ChartWidget.h"
#include "data/interfaces/IDataRepository.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGridLayout>
#include <limits>

ChartWidget::ChartWidget(QWidget* parent)
    : QWidget(parent)
    , m_repository(nullptr)
    , m_timeRange(300)
    , m_recording(false)
    // , m_mainSplitter(new QSplitter(Qt::Horizontal, this))
    // , m_speedometerPanel(new QWidget())
    // , m_speedometerAD(new SpeedometerWidget("АД", 0, 3000))
    // , m_speedometerTK(new SpeedometerWidget("ТК", 0, 15000))
    // , m_speedometerST(new SpeedometerWidget("СТ", 0, 20000))
    , m_chartPanel(new QWidget())
    , m_chart(new QChart())
    , m_chartView(new QChartView(m_chart))
    , m_seriesAD(new QLineSeries())
    , m_seriesTK(new QLineSeries())
    , m_seriesST(new QLineSeries())
    , m_axisX(new QDateTimeAxis())
    , m_axisY_AD(new QValueAxis())
    , m_axisY_TK(new QValueAxis())
    , m_axisY_ST(new QValueAxis())
    , m_autoScaleAD(new QCheckBox("Авто"))
    , m_autoScaleTK(new QCheckBox("Авто"))
    , m_autoScaleST(new QCheckBox("Авто"))
    , m_minAD(new QDoubleSpinBox())
    , m_maxAD(new QDoubleSpinBox())
    , m_minTK(new QDoubleSpinBox())
    , m_maxTK(new QDoubleSpinBox())
    , m_minST(new QDoubleSpinBox())
    , m_maxST(new QDoubleSpinBox())
    , m_timeRangeSpin(new QSpinBox())
    , m_showGrid(new QCheckBox("Показать сетку"))
    , m_resetZoomButton(new QPushButton("Сброс zoom"))
{
    setupChart();
    setupMultiAxis();
    setupControlPanel();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Настройка разделителя
    // m_mainSplitter->addWidget(m_speedometerPanel);
    // m_mainSplitter->addWidget(m_chartPanel);
    // m_mainSplitter->setSizes({400, 800}); // 400px спидометры, 800px графики

    mainLayout->addWidget(m_chartPanel);
    setLayout(mainLayout);

    // Подключение сигналов
    connect(m_autoScaleAD, &QCheckBox::stateChanged, this, &ChartWidget::onAutoScaleADChanged);
    connect(m_autoScaleTK, &QCheckBox::stateChanged, this, &ChartWidget::onAutoScaleTKChanged);
    connect(m_autoScaleST, &QCheckBox::stateChanged, this, &ChartWidget::onAutoScaleSTChanged);
    connect(m_minAD, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleADChanged);
    connect(m_maxAD, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleADChanged);
    connect(m_minTK, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleTKChanged);
    connect(m_maxTK, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleTKChanged);
    connect(m_minST, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleSTChanged);
    connect(m_maxST, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &ChartWidget::onManualScaleSTChanged);
    connect(m_timeRangeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChartWidget::onTimeRangeChanged);
    connect(m_showGrid, &QCheckBox::toggled, this, &ChartWidget::onGridToggled);
    connect(m_resetZoomButton, &QPushButton::clicked, this, &ChartWidget::resetZoom);
}

void ChartWidget::setupControlPanel() {
    // Настройка правой панели с графиками
    QVBoxLayout* chartLayout = new QVBoxLayout(m_chartPanel);
    chartLayout->setContentsMargins(0, 0, 0, 0);

    // Заголовок панели графиков
    QLabel* chartTitle = new QLabel("ГРАФИКИ ОБОРОТОВ ВО ВРЕМЕНИ");
    chartTitle->setAlignment(Qt::AlignCenter);
    QFont titleFont = chartTitle->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    chartTitle->setFont(titleFont);
    chartLayout->addWidget(chartTitle);

    chartLayout->addWidget(m_chartView);

    // Панель управления масштабом и отображением
    QGroupBox* controlGroup = new QGroupBox("Управление масштабом и отображением");
    QGridLayout* controlLayout = new QGridLayout(controlGroup);

    // Масштаб по времени
    QGroupBox* timeGroup = new QGroupBox("Время (сек)");
    QVBoxLayout* timeLayout = new QVBoxLayout(timeGroup);
    m_timeRangeSpin->setRange(10, 3600);
    m_timeRangeSpin->setValue(m_timeRange);
    m_timeRangeSpin->setSuffix(" сек");
    timeLayout->addWidget(m_timeRangeSpin);
    controlLayout->addWidget(timeGroup, 0, 0);

    // Масштаб АД
    QGroupBox* adGroup = new QGroupBox("АД об/мин");
    QFormLayout* adLayout = new QFormLayout(adGroup);
    adLayout->addRow("Авто:", m_autoScaleAD);
    adLayout->addRow("Мин:", m_minAD);
    adLayout->addRow("Макс:", m_maxAD);
    controlLayout->addWidget(adGroup, 0, 1);

    // Масштаб ТК
    QGroupBox* tkGroup = new QGroupBox("ТК об/мин");
    QFormLayout* tkLayout = new QFormLayout(tkGroup);
    tkLayout->addRow("Авто:", m_autoScaleTK);
    tkLayout->addRow("Мин:", m_minTK);
    tkLayout->addRow("Макс:", m_maxTK);
    controlLayout->addWidget(tkGroup, 0, 2);

    // Масштаб СТ
    QGroupBox* stGroup = new QGroupBox("СТ об/мин");
    QFormLayout* stLayout = new QFormLayout(stGroup);
    stLayout->addRow("Авто:", m_autoScaleST);
    stLayout->addRow("Мин:", m_minST);
    stLayout->addRow("Макс:", m_maxST);
    controlLayout->addWidget(stGroup, 0, 3);

    // Управление отображением
    QGroupBox* displayGroup = new QGroupBox("Отображение");
    QVBoxLayout* displayLayout = new QVBoxLayout(displayGroup);
    displayLayout->addWidget(m_showGrid);
    displayLayout->addWidget(m_resetZoomButton);
    controlLayout->addWidget(displayGroup, 0, 4);

    chartLayout->addWidget(controlGroup);

    // Настройка спинбоксов
    setupControlPanelValues();
}

void ChartWidget::setupControlPanelValues() {
    // Настройка спинбоксов для АД
    m_minAD->setRange(-100000, 100000);
    m_maxAD->setRange(-100000, 100000);
    m_minAD->setValue(0);
    m_maxAD->setValue(3000);
    m_minAD->setSingleStep(100);
    m_maxAD->setSingleStep(100);

    // Настройка спинбоксов для ТК
    m_minTK->setRange(-100000, 100000);
    m_maxTK->setRange(-100000, 100000);
    m_minTK->setValue(0);
    m_maxTK->setValue(71500);
    m_minTK->setSingleStep(1500);
    m_maxTK->setSingleStep(1500);

    // Настройка спинбоксов для СТ
    m_minST->setRange(-100000, 100000);
    m_maxST->setRange(-100000, 100000);
    m_minST->setValue(0);
    m_maxST->setValue(65000);
    m_minST->setSingleStep(1500);
    m_maxST->setSingleStep(1500);

    // По умолчанию авто масштабирование
    m_autoScaleAD->setChecked(true);
    m_autoScaleTK->setChecked(true);
    m_autoScaleST->setChecked(true);

    // Сетка по умолчанию выключена
    m_showGrid->setChecked(false);
}

void ChartWidget::setupChart() {
    // Настройка серий
    m_seriesAD->setName("АД об/мин");
    m_seriesAD->setColor(Qt::blue);

    m_seriesTK->setName("ТК об/мин");
    m_seriesTK->setColor(Qt::red);

    m_seriesST->setName("СТ об/мин");
    m_seriesST->setColor(Qt::green);

    m_chart->addSeries(m_seriesAD);
    m_chart->addSeries(m_seriesTK);
    m_chart->addSeries(m_seriesST);

    m_chart->setTitle("Мониторинг оборотов во времени");
    m_chart->legend()->setVisible(true);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    // Включаем zoom и скроллинг
    m_chartView->setRubberBand(QChartView::RectangleRubberBand);
    m_chartView->setInteractive(true);
}

void ChartWidget::setupMultiAxis() {
    // Ось X
    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("Время");
    m_axisX->setGridLineVisible(false);
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    // Ось Y для АД (синяя)
    m_axisY_AD->setTitleText("АД об/мин");
    m_axisY_AD->setLinePenColor(Qt::blue);
    m_axisY_AD->setLabelsColor(Qt::blue);
    m_axisY_AD->setTitleBrush(QBrush(Qt::blue));
    m_axisY_AD->setGridLineVisible(false);
    m_chart->addAxis(m_axisY_AD, Qt::AlignLeft);
    m_seriesAD->attachAxis(m_axisY_AD);

    // Ось Y для ТК (красная) - справа
    m_axisY_TK->setTitleText("ТК об/мин");
    m_axisY_TK->setLinePenColor(Qt::red);
    m_axisY_TK->setLabelsColor(Qt::red);
    m_axisY_TK->setTitleBrush(QBrush(Qt::red));
    m_axisY_TK->setGridLineVisible(false);
    m_chart->addAxis(m_axisY_TK, Qt::AlignRight);
    m_seriesTK->attachAxis(m_axisY_TK);

    // Ось Y для СТ (зеленая) - справа со смещением
    m_axisY_ST->setTitleText("СТ об/мин");
    m_axisY_ST->setLinePenColor(Qt::green);
    m_axisY_ST->setLabelsColor(Qt::green);
    m_axisY_ST->setTitleBrush(QBrush(Qt::green));
    m_axisY_ST->setGridLineVisible(false);
    m_chart->addAxis(m_axisY_ST, Qt::AlignRight);
    m_seriesST->attachAxis(m_axisY_ST);

    // Все серии прикрепляем к оси X
    m_seriesAD->attachAxis(m_axisX);
    m_seriesTK->attachAxis(m_axisX);
    m_seriesST->attachAxis(m_axisX);
}

void ChartWidget::updateGrid() {
    bool showGrid = m_showGrid->isChecked();

    // Включаем/выключаем основную сетку графика
    m_chart->setBackgroundVisible(showGrid);

    // Настраиваем сетку для осей
    QPen gridPen(QColor(200, 200, 200, 100)); // Светло-серый с прозрачностью
    gridPen.setStyle(Qt::DashLine);

    // Ось X
    m_axisX->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisX->setGridLinePen(gridPen);
        m_axisX->setShadesVisible(true);
        m_axisX->setShadesBrush(QBrush(QColor(240, 240, 240, 50)));
    }

    // Ось Y АД
    m_axisY_AD->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisY_AD->setGridLinePen(gridPen);
    }

    // Ось Y ТК
    m_axisY_TK->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisY_TK->setGridLinePen(gridPen);
    }

    // Ось Y СТ
    m_axisY_ST->setGridLineVisible(showGrid);
    if (showGrid) {
        m_axisY_ST->setGridLinePen(gridPen);
    }
}

// Слоты для управления масштабом
void ChartWidget::onAutoScaleADChanged(int state) {
    m_minAD->setEnabled(state == Qt::Unchecked);
    m_maxAD->setEnabled(state == Qt::Unchecked);
    if (state == Qt::Checked) {
        updateChart(); // Пересчитать авто масштаб
    }
}

void ChartWidget::onAutoScaleTKChanged(int state) {
    m_minTK->setEnabled(state == Qt::Unchecked);
    m_maxTK->setEnabled(state == Qt::Unchecked);
    if (state == Qt::Checked) {
        updateChart();
    }
}

void ChartWidget::onAutoScaleSTChanged(int state) {
    m_minST->setEnabled(state == Qt::Unchecked);
    m_maxST->setEnabled(state == Qt::Unchecked);
    if (state == Qt::Checked) {
        updateChart();
    }
}

void ChartWidget::onManualScaleADChanged() {
    if (!m_autoScaleAD->isChecked()) {
        m_axisY_AD->setRange(m_minAD->value(), m_maxAD->value());
    }
}

void ChartWidget::onManualScaleTKChanged() {
    if (!m_autoScaleTK->isChecked()) {
        m_axisY_TK->setRange(m_minTK->value(), m_maxTK->value());
    }
}

void ChartWidget::onManualScaleSTChanged() {
    if (!m_autoScaleST->isChecked()) {
        m_axisY_ST->setRange(m_minST->value(), m_maxST->value());
    }
}

void ChartWidget::onTimeRangeChanged() {
    m_timeRange = m_timeRangeSpin->value();
    updateChart();
}

void ChartWidget::onGridToggled(bool enabled) {
    updateGrid();
}

void ChartWidget::resetZoom() {
    // Сбрасываем zoom к исходному масштабу
    m_chart->zoomReset();

    // Восстанавливаем авто масштабирование если нужно
    if (m_autoScaleAD->isChecked() || m_autoScaleTK->isChecked() || m_autoScaleST->isChecked()) {
        updateChart();
    }
}

void ChartWidget::applyManualScale() {
    if (!m_autoScaleAD->isChecked()) {
        m_axisY_AD->setRange(m_minAD->value(), m_maxAD->value());
    }
    if (!m_autoScaleTK->isChecked()) {
        m_axisY_TK->setRange(m_minTK->value(), m_maxTK->value());
    }
    if (!m_autoScaleST->isChecked()) {
        m_axisY_ST->setRange(m_minST->value(), m_maxST->value());
    }
}

void ChartWidget::startTestRecording() {
    m_recording = true;
    m_chart->setTitle("Мониторинг оборотов [ЗАПИСЬ]");

    // ОЧИЩАЕМ ДАННЫЕ РЕПОЗИТОРИЯ ПРИ НАЧАЛЕ ТЕСТА
    if (m_repository) {
        m_repository->clearData("AD_RPM");
        m_repository->clearData("TK_RPM");
        m_repository->clearData("ST_RPM");
        m_repository->clearData("TK_PERCENT");
        m_repository->clearData("ST_PERCENT");
    }

    // Очищаем предыдущие данные
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();

    // Сбрасываем масштаб если авто
    if (m_autoScaleAD->isChecked() || m_autoScaleTK->isChecked() || m_autoScaleST->isChecked()) {
        updateChart();
    }
}

void ChartWidget::stopTestRecording() {
    m_recording = false;
    m_chart->setTitle("Мониторинг оборотов [ОСТАНОВЛЕНО]");
}

void ChartWidget::clearChart() {
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();
    updateChart();
}

void ChartWidget::loadHistoricalData(const QVector<DataPointRecord>& points) {
    // Очищаем текущие данные
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();

    // Группируем точки по параметрам
    QVector<DataPointRecord> adPoints, tkPoints, stPoints;

    for (const auto& point : points) {
        if (point.parameter == "AD_RPM") {
            adPoints.append(point);
        } else if (point.parameter == "TK_RPM") {
            tkPoints.append(point);
        } else if (point.parameter == "ST_RPM") {
            stPoints.append(point);
        }
    }

    // Добавляем точки в серии
    for (const auto& point : adPoints) {
        m_seriesAD->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }
    for (const auto& point : tkPoints) {
        m_seriesTK->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }
    for (const auto& point : stPoints) {
        m_seriesST->append(point.timestamp.toMSecsSinceEpoch(), point.value);
    }

    // Обновляем масштаб
    updateChart();

    m_chart->setTitle("Исторические данные теста");
}

void ChartWidget::setDataRepository(IDataRepository* repository) {
    m_repository = repository;
    if (m_repository) {
        connect(m_repository, &IDataRepository::dataAdded,
                this, &ChartWidget::updateChart);
    }
}

void ChartWidget::setParameter(const QString& parameter) {
    m_parameter = parameter;
    m_chart->setTitle(QString("Chart for %1").arg(parameter));
    updateChart();
}

void ChartWidget::updateChart() {
    if (!m_repository || !m_recording) {
        return;
    }

    QDateTime to = QDateTime::currentDateTime();
    QDateTime from = to.addSecs(-m_timeRange);

    // Получаем данные для всех трех параметров
    auto pointsAD = m_repository->getDataPoints("AD_RPM", from, to);
    auto pointsTK = m_repository->getDataPoints("TK_RPM", from, to);
    auto pointsST = m_repository->getDataPoints("ST_RPM", from, to);

    // // Обновляем спидометры последними значениями
    // if (!pointsAD.isEmpty()) {
    //     double lastAD = pointsAD.last().value;
    //     // m_speedometerAD->setValue(lastAD);
    // }
    // if (!pointsTK.isEmpty()) {
    //     double lastTK = pointsTK.last().value;
    //     m_speedometerTK->setValue(lastTK);
    // }
    // if (!pointsST.isEmpty()) {
    //     double lastST = pointsST.last().value;
    //     m_speedometerST->setValue(lastST);
    // }

    // Обновляем графики (существующий код)
    m_seriesAD->clear();
    m_seriesTK->clear();
    m_seriesST->clear();

    double minAD = std::numeric_limits<double>::max();
    double maxAD = std::numeric_limits<double>::min();
    double minTK = std::numeric_limits<double>::max();
    double maxTK = std::numeric_limits<double>::min();
    double minST = std::numeric_limits<double>::max();
    double maxST = std::numeric_limits<double>::min();

    // Добавляем точки для АД
    for (const auto& point : pointsAD) {
        qint64 timestamp = point.timestamp.toMSecsSinceEpoch();
        m_seriesAD->append(timestamp, point.value);
        minAD = qMin(minAD, point.value);
        maxAD = qMax(maxAD, point.value);
    }

    // Добавляем точки для ТК
    for (const auto& point : pointsTK) {
        qint64 timestamp = point.timestamp.toMSecsSinceEpoch();
        m_seriesTK->append(timestamp, point.value);
        minTK = qMin(minTK, point.value);
        maxTK = qMax(maxTK, point.value);
    }

    // Добавляем точки для СТ
    for (const auto& point : pointsST) {
        qint64 timestamp = point.timestamp.toMSecsSinceEpoch();
        m_seriesST->append(timestamp, point.value);
        minST = qMin(minST, point.value);
        maxST = qMax(maxST, point.value);
    }

    // Автомасштабирование осей если включено
    if (m_autoScaleAD->isChecked() && !pointsAD.isEmpty()) {
        double rangeAD = maxAD - minAD;
        if (rangeAD < 1.0) rangeAD = 1.0;
        m_axisY_AD->setRange(minAD - rangeAD * 0.1, maxAD + rangeAD * 0.1);
        m_minAD->setValue(minAD - rangeAD * 0.1);
        m_maxAD->setValue(maxAD + rangeAD * 0.1);
    }

    if (m_autoScaleTK->isChecked() && !pointsTK.isEmpty()) {
        double rangeTK = maxTK - minTK;
        if (rangeTK < 1.0) rangeTK = 1.0;
        m_axisY_TK->setRange(minTK - rangeTK * 0.1, maxTK + rangeTK * 0.1);
        m_minTK->setValue(minTK - rangeTK * 0.1);
        m_maxTK->setValue(maxTK + rangeTK * 0.1);
    }

    if (m_autoScaleST->isChecked() && !pointsST.isEmpty()) {
        double rangeST = maxST - minST;
        if (rangeST < 1.0) rangeST = 1.0;
        m_axisY_ST->setRange(minST - rangeST * 0.1, maxST + rangeST * 0.1);
        m_minST->setValue(minST - rangeST * 0.1);
        m_maxST->setValue(maxST + rangeST * 0.1);
    }

    // Применяем ручной масштаб если нужно
    applyManualScale();

    // Обновляем ось X если есть данные
    if (!pointsAD.isEmpty() || !pointsTK.isEmpty() || !pointsST.isEmpty()) {
        m_axisX->setRange(from, to);
    }
}
