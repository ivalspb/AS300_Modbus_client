#include "ModbusRequestHandler.h"
#include "core/interfaces/IRequestQueue.h"
#include <QModbusDataUnit>
#include <QDebug>

ModbusRequestHandler::ModbusRequestHandler(QModbusTcpClient* client,
                                           IRequestQueue* queue,
                                           QObject* parent)
    : QObject(parent)
    , m_client(client)
    , m_queue(queue)
    , m_processTimer(new QTimer(this))
    , m_processing(false)
    , m_minRequestInterval(50)
{
    m_processTimer->setInterval(m_minRequestInterval);
    connect(m_processTimer, &QTimer::timeout, this, &ModbusRequestHandler::processNextRequest);
}

ModbusRequestHandler::~ModbusRequestHandler() {
    stop();
    if (m_processTimer) {
        m_processTimer->stop();
    }
}

void ModbusRequestHandler::setProcessInterval(int ms) {
    m_minRequestInterval = qMax(10, ms);
    m_processTimer->setInterval(m_minRequestInterval);
}

void ModbusRequestHandler::start() {
    if (!m_processTimer->isActive()) {
        m_processTimer->start();
    }
}

void ModbusRequestHandler::stop() {
    m_processTimer->stop();
    m_processing = false;
}

void ModbusRequestHandler::processNextRequest() {
//    qDebug() << "RequestHandler: processNextRequest called - Processing:" << m_processing;

    if (m_processing) {
//        qDebug() << "RequestHandler: Already processing, skipping";
        return;
    }

    if (!m_queue->hasRequests()) {
//        qDebug() << "RequestHandler: No requests in queue";
        return;
    }

    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
//        qDebug() << "RequestHandler: Client not ready - State:" << (m_client ? m_client->state() : -1);
        return;
    }

    m_processing = true;
    ModbusRequest request = m_queue->dequeue();

    if (request.type == RequestType::Read) {
//        qDebug() << "RequestHandler: Processing READ request";
        sendReadRequest(request.registerType, request.address, request.count, request.parameterName);
    } else {
//        qDebug() << "RequestHandler: Processing WRITE request";
        sendWriteRequest(request.registerType, request.address, request.value);
    }
}

void ModbusRequestHandler::sendReadRequest(QModbusDataUnit::RegisterType type,
                                         quint16 address, quint16 count,
                                         const QString& paramName) {
    QModbusDataUnit readUnit(type, address, count);
    QModbusReply* reply = m_client->sendReadRequest(readUnit, 1);

    if (!reply) {
        m_processing = false;
        emit requestFailed("Failed to create read request");
        return;
    }

    reply->setProperty("address", address);
    reply->setProperty("count", count);
    reply->setProperty("paramName", paramName);
    reply->setProperty("registerType", static_cast<int>(type));

    connect(reply, &QModbusReply::finished, this, &ModbusRequestHandler::handleReadReply);
}

void ModbusRequestHandler::sendWriteRequest(QModbusDataUnit::RegisterType type,
                                            quint16 address, quint16 value) {
//    qDebug() << "ModbusRequestHandler: Sending write request - Type:" << type
//             << "Address: 0x" << QString::number(address, 16)
//             << "Value:" << value
//             << "Client state:" << m_client->state();

    if (!m_client || m_client->state() != QModbusDevice::ConnectedState) {
        QString errorMsg = "Client not in connected state: " + QString::number(m_client->state());
//        qDebug() << "ModbusRequestHandler:" << errorMsg;
        m_processing = false;
        emit requestFailed(errorMsg);
        return;
    }

    QModbusDataUnit writeUnit(type, address, 1);

    // Для битовых регистров преобразуем значение
    if (type == QModbusDataUnit::Coils || type == QModbusDataUnit::DiscreteInputs) {
        writeUnit.setValue(0, value > 0 ? true : false);
//        qDebug() << "Coil write - setting value to:" << (value > 0 ? "true" : "false");
    } else {
        writeUnit.setValue(0, value);
//        qDebug() << "Register write - setting value to:" << value;
    }

    QModbusReply* reply = m_client->sendWriteRequest(writeUnit, 1);
//    qDebug() << "RequestHandler: Write request sent, reply:" << reply;

    if (!reply) {
        m_processing = false;
        QString errorMsg = "Failed to create write request for address: 0x" + QString::number(address, 16);
//        qDebug() << "ModbusRequestHandler:" << errorMsg;
        emit requestFailed(errorMsg);
        return;
    }

    reply->setProperty("address", address);
    reply->setProperty("value", value);
    reply->setProperty("registerType", static_cast<int>(type));

    connect(reply, &QModbusReply::finished, this, &ModbusRequestHandler::handleWriteReply);
    connect(reply, &QModbusReply::errorOccurred, this, [this, address, reply](QModbusDevice::Error error) {
        qDebug() << "ModbusRequestHandler: Write error for address 0x" << QString::number(address, 16)
                 << "Error:" << error << "Error string:" << reply->errorString();
    });

//    qDebug() << "ModbusRequestHandler: Write request sent successfully, reply:" << reply;
}

void ModbusRequestHandler::handleReadReply() {
    QModbusReply* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) return;

    quint16 address = reply->property("address").toUInt();
    quint16 count = reply->property("count").toUInt();
    QString paramName = reply->property("paramName").toString();
    QModbusDataUnit::RegisterType registerType = static_cast<QModbusDataUnit::RegisterType>(
        reply->property("registerType").toInt());

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();

        // Для всех типов регистров возвращаем quint16 значения
        if (count == 1) {
            quint16 value = unit.value(0);
            emit readCompleted(registerType, address, value, paramName);
        } else {
            QVector<quint16> values;
            for (quint16 i = 0; i < unit.valueCount(); ++i) {
                values.append(unit.value(i));
            }
            emit readsCompleted(registerType, address, values);
        }
    } else {
        emit requestFailed("Read error: " + reply->errorString());
    }

    reply->deleteLater();
    m_processing = false;
}

void ModbusRequestHandler::handleWriteReply() {
    QModbusReply* reply = qobject_cast<QModbusReply*>(sender());
    if (!reply) {
//        qDebug() << "RequestHandler: handleWriteReply - No reply object!";
        m_processing = false;
        return;
    }

    quint16 address = reply->property("address").toUInt();
    QModbusDataUnit::RegisterType registerType = static_cast<QModbusDataUnit::RegisterType>(
        reply->property("registerType").toInt());
    bool success = (reply->error() == QModbusDevice::NoError);

    emit writeCompleted(registerType, address, success);

    if (!success) {
        QString errorMsg = "Write error: " + reply->errorString();
//        qDebug() << "RequestHandler:" << errorMsg;
        emit requestFailed(errorMsg);
    }

    reply->deleteLater();
    m_processing = false;
//    qDebug() << "RequestHandler: Write processing completed";
}
