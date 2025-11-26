#pragma once
#include "IDatabaseRepository.h"
#include "export/interfaces/IExportStrategy.h"
#include <QObject>

class DatabaseExportService : public QObject {
    Q_OBJECT
public:
    explicit DatabaseExportService(IDatabaseRepository* repository, QObject* parent = nullptr);

    void exportSessionToImage(int sessionId, const QString& filename, IExportStrategy* exportStrategy);
    void exportSessionToCsv(int sessionId, const QString& filename);

signals:
    void exportCompleted(const QString& filename);
    void exportFailed(const QString& error);

private:
    IDatabaseRepository* m_repository;
};
