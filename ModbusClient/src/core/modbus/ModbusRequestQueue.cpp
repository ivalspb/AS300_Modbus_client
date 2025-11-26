#include "ModbusRequestQueue.h"
#include <QMutexLocker>
#include <QDebug>

ModbusRequestQueue::ModbusRequestQueue(QObject* parent)
    : IRequestQueue(parent)
{}

void ModbusRequestQueue::enqueueRead(QModbusDataUnit::RegisterType type, quint16 address,
                                   quint16 count, const QString& paramName) {
    QMutexLocker locker(&m_mutex);

    ModbusRequest request(RequestType::Read, type, address, count, 0, paramName);
    m_normalQueue.enqueue(request);
//    qDebug() << "Queue: Enqueued READ - Type:" << type
//             << "Address: 0x" << QString::number(address, 16)
//             << "Count:" << count
//             << "Queue size:" << m_normalQueue.size();
    locker.unlock();
    emit requestAdded();
}

void ModbusRequestQueue::enqueueWrite(QModbusDataUnit::RegisterType type, quint16 address,
                                    quint16 value) {
    QMutexLocker locker(&m_mutex);

    ModbusRequest request(RequestType::Write, type, address, 1, value);
    m_priorityQueue.enqueue(request);
//    qDebug() << "Queue: Enqueued WRITE - Type:" << type
//             << "Address: 0x" << QString::number(address, 16)
//             << "Value:" << value
//             << "Queue size:" << m_priorityQueue.size();

    locker.unlock();
    emit requestAdded();
}

void ModbusRequestQueue::enqueuePriorityRead(QModbusDataUnit::RegisterType type, quint16 address,
                                           quint16 count, const QString& paramName) {
    QMutexLocker locker(&m_mutex);

    ModbusRequest request(RequestType::Read, type, address, count, 0, paramName);
    m_priorityQueue.enqueue(request); // Приоритетные чтения в приоритетную очередь

    locker.unlock();
    emit requestAdded();
}

bool ModbusRequestQueue::hasRequests() const {
    QMutexLocker locker(&m_mutex);
    bool hasRequests = !m_priorityQueue.isEmpty()|| !m_normalQueue.isEmpty();
//    qDebug() << "m_priorityQueue: hasRequests =" << hasRequests << "Size:" << m_priorityQueue.size();
//    qDebug() << "m_normalQueue: hasRequests =" << hasRequests << "Size:" << m_normalQueue.size();
    return hasRequests;
}

ModbusRequest ModbusRequestQueue::dequeue() {
    QMutexLocker locker(&m_mutex);

    if (m_priorityQueue.isEmpty()) {
//        qDebug() << "Queue: Attempt to dequeue from empty m_priorityQueue!";
        ModbusRequest request = m_normalQueue.dequeue();
//        qDebug() << "m_normalQueue: Dequeued - Type:" << (request.type == RequestType::Read ? "READ" : "WRITE")
//                 << "Address: 0x" << QString::number(request.address, 16)
//                 << "Remaining:" << m_normalQueue.size();
        return request;
    }
    else {
        ModbusRequest request = m_priorityQueue.dequeue();
//        qDebug() << "Queue: Dequeued - Type:" << (request.type == RequestType::Read ? "READ" : "WRITE")
//                 << "Address: 0x" << QString::number(request.address, 16)
//                 << "Remaining:" << m_priorityQueue.size();
        return request;
    }
}

void ModbusRequestQueue::clear() {
    QMutexLocker locker(&m_mutex);
    int previousSize = m_priorityQueue.size();
    m_priorityQueue.clear();
//    qDebug() << "m_priorityQueue: Cleared - Previous size:" << previousSize;

    previousSize = m_normalQueue.size();
    m_normalQueue.clear();
//    qDebug() << "m_priorityQueue: Cleared - Previous size:" << previousSize;
}

int ModbusRequestQueue::size() const {
    QMutexLocker locker(&m_mutex);
    return m_priorityQueue.size() + m_normalQueue.size();
}
