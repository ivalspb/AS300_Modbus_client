#pragma once
#include <QObject>
#include <functional>
#include <QModbusDataUnit>

enum class RequestType {
    Read,
    Write
};

struct ModbusRequest {
    RequestType type;
    QModbusDataUnit::RegisterType registerType;
    quint16 address;
    quint16 count; // for read operations
    quint16 value; // for write operations
    QString parameterName;
    std::function<void(bool, const QString&)> callback;

    ModbusRequest() = default;

    ModbusRequest(RequestType t, QModbusDataUnit::RegisterType rt, quint16 addr,
                  quint16 cnt = 1, quint16 val = 0, const QString& param = "")
        : type(t), registerType(rt), address(addr), count(cnt), value(val), parameterName(param)
    {}
};

class IRequestQueue : public QObject {
    Q_OBJECT
public:
    explicit IRequestQueue(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IRequestQueue() = default;

    virtual void enqueueRead(QModbusDataUnit::RegisterType type, quint16 address,
                            quint16 count = 1, const QString& paramName = "") = 0;
    virtual void enqueuePriorityRead(QModbusDataUnit::RegisterType type, quint16 address,
                                     quint16 count, const QString& paramName = "") = 0;

    virtual void enqueueWrite(QModbusDataUnit::RegisterType type, quint16 address,
                             quint16 value) = 0;
    virtual bool hasRequests() const = 0;
    virtual ModbusRequest dequeue() = 0;
    virtual void clear() = 0;
    virtual int size() const = 0;

signals:
    void requestAdded();
};
