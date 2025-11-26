#include "DatabaseExportService.h"
#include "gui/widgets/ChartWidget.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

DatabaseExportService::DatabaseExportService(IDatabaseRepository* repository, QObject* parent)
    : QObject(parent)
    , m_repository(repository)
{}

void DatabaseExportService::exportSessionToImage(int sessionId, const QString& filename, IExportStrategy* exportStrategy) {
    try {
        // Загружаем данные сессии
        auto adPoints = m_repository->getDataPoints(sessionId, "AD_RPM");
        auto tkPoints = m_repository->getDataPoints(sessionId, "TK_RPM");
        auto stPoints = m_repository->getDataPoints(sessionId, "ST_RPM");

        if (adPoints.isEmpty() && tkPoints.isEmpty() && stPoints.isEmpty()) {
            emit exportFailed("No data available for export");
            return;
        }

        // Создаем временный виджет графика
        ChartWidget* chartWidget = new ChartWidget();

        // Добавляем данные в график
        // Здесь нужно добавить методы в ChartWidget для загрузки исторических данных
        // chartWidget->loadHistoricalData(adPoints, tkPoints, stPoints);

        // Экспортируем
        if (exportStrategy->exportWidget(chartWidget, filename)) {
            emit exportCompleted(filename);
        } else {
            emit exportFailed("Failed to export chart to image");
        }

        chartWidget->deleteLater();

    } catch (const std::exception& e) {
        emit exportFailed(QString("Export error: %1").arg(e.what()));
    }
}

void DatabaseExportService::exportSessionToCsv(int sessionId, const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit exportFailed("Cannot open file for writing");
        return;
    }

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    // Заголовок CSV
    stream << "Timestamp,AD_RPM,TK_RPM,ST_RPM,TK_PERCENT,ST_PERCENT\n";

    try {
        // Загружаем все данные сессии
        auto adPoints = m_repository->getDataPoints(sessionId, "AD_RPM");
        auto tkPoints = m_repository->getDataPoints(sessionId, "TK_RPM");
        auto stPoints = m_repository->getDataPoints(sessionId, "ST_RPM");
        auto tkPercentPoints = m_repository->getDataPoints(sessionId, "TK_PERCENT");
        auto stPercentPoints = m_repository->getDataPoints(sessionId, "ST_PERCENT");

        // Собираем все временные метки
        QSet<QDateTime> allTimestamps;
        for (const auto& point : adPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : tkPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : stPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : tkPercentPoints) allTimestamps.insert(point.timestamp);
        for (const auto& point : stPercentPoints) allTimestamps.insert(point.timestamp);

        // Сортируем временные метки
        QList<QDateTime> sortedTimestamps = allTimestamps.values();
        std::sort(sortedTimestamps.begin(), sortedTimestamps.end());

        // Создаем карты для быстрого доступа к значениям по времени
        QMap<QDateTime, double> adMap, tkMap, stMap, tkPercentMap, stPercentMap;
        for (const auto& point : adPoints) adMap[point.timestamp] = point.value;
        for (const auto& point : tkPoints) tkMap[point.timestamp] = point.value;
        for (const auto& point : stPoints) stMap[point.timestamp] = point.value;
        for (const auto& point : tkPercentPoints) tkPercentMap[point.timestamp] = point.value;
        for (const auto& point : stPercentPoints) stPercentMap[point.timestamp] = point.value;

        // Записываем данные
        for (const auto& timestamp : sortedTimestamps) {
            stream << timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz") << ","
                   << adMap.value(timestamp, 0) << ","
                   << tkMap.value(timestamp, 0) << ","
                   << stMap.value(timestamp, 0) << ","
                   << tkPercentMap.value(timestamp, 0) << ","
                   << stPercentMap.value(timestamp, 0) << "\n";
        }

        file.close();
        emit exportCompleted(filename);

    } catch (const std::exception& e) {
        file.close();
        emit exportFailed(QString("CSV export error: %1").arg(e.what()));
    }
}
