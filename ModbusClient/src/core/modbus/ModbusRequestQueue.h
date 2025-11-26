#pragma once
#include "core/interfaces/IRequestQueue.h"
#include <QQueue>
#include <QMutex>

class ModbusRequestQueue : public IRequestQueue {
    Q_OBJECT
public:
    explicit ModbusRequestQueue(QObject* parent = nullptr);
    ~ModbusRequestQueue() override = default;

    void enqueueRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 count, const QString& paramName) override;
    void enqueueWrite(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) override;
    void enqueuePriorityRead(QModbusDataUnit::RegisterType type, quint16 address, quint16 count, const QString& paramName) override;
    bool hasRequests() const override;
    ModbusRequest dequeue() override;
    void clear() override;
    int size() const override;

private:
    mutable QMutex m_mutex;
    QQueue<ModbusRequest> m_priorityQueue; // Приоритетная очередь для записей и проверок
    QQueue<ModbusRequest> m_normalQueue;   // Обычная очередь для опросов
};
