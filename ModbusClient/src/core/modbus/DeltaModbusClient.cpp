#include "DeltaModbusClient.h"
#include "CustomModbusClient.h"
#include "ModbusRequestQueue.h"
#include "ModbusRequestHandler.h"
#include "core/mapping/DeltaAddressMapper.h"
#include "core/mapping/DeltaAddressMap.h"
#include <QDebug>
#include <QThread>
#include <QDateTime>

DeltaModbusClient::DeltaModbusClient(QObject* parent)
    : IModbusClient(parent)
    , m_client(new CustomModbusClient(this))
    , m_queue(new ModbusRequestQueue(this))
    , m_handler(new ModbusRequestHandler(m_client.data(), m_queue.data(), this))
    , m_mapper(new DeltaAddressMapper())
    , m_verificationTimer(new QTimer(this))
    , m_currentMode("Холодная прокрутка турбостартера")
    , m_localPort(3201)
    , m_initialD0Written(false)
{
    // Configure Modbus client
    m_client->setTimeout(5000);
    m_client->setNumberOfRetries(2);

    setupPollingTimers();
    setupPollingGroups();

    // Connect client signals
    connect(m_client.data(), &QModbusClient::stateChanged,
            this, &DeltaModbusClient::onStateChanged);
    connect(m_client.data(), &QModbusClient::errorOccurred,
            this, &DeltaModbusClient::onErrorOccurred);

    // Connect handler signals
    connect(m_handler.data(), SIGNAL(readCompleted(QModbusDataUnit::RegisterType,quint16,quint16,QString)),
            this, SLOT(onReadCompleted(QModbusDataUnit::RegisterType,quint16,quint16,QString)));
    connect(m_handler.data(), SIGNAL(readsCompleted(QModbusDataUnit::RegisterType,quint16,QVector<quint16>)),
            this, SLOT(onReadsCompleted(QModbusDataUnit::RegisterType,quint16,QVector<quint16>)));
    connect(m_handler.data(), SIGNAL(writeCompleted(QModbusDataUnit::RegisterType,quint16,bool)),
            this, SLOT(onWriteCompleted(QModbusDataUnit::RegisterType,quint16,bool)));

    connect(m_handler.data(), &ModbusRequestHandler::requestFailed,
            this, &DeltaModbusClient::onRequestFailed);

    // Verification timer
    m_verificationTimer->setInterval(500);
    connect(m_verificationTimer, &QTimer::timeout, this, &DeltaModbusClient::onVerificationTimeout);

    m_initialD0Written = false;
}

DeltaModbusClient::~DeltaModbusClient() {
    // Останавливаем все таймеры опроса
    for (QTimer* timer : m_pollTimers) {
        if (timer) {
            timer->stop();
        }
    }

    if (m_handler) {
        m_handler->stop();
    }

    disconnectFromDevice();
}

void DeltaModbusClient::setupPollingTimers() {
    // High frequency timer (20 Hz = 50 ms)
    m_pollTimers[HIGH_FREQUENCY] = new QTimer(this);
    m_pollTimers[HIGH_FREQUENCY]->setInterval(50);
    connect(m_pollTimers[HIGH_FREQUENCY], &QTimer::timeout,
            this, [this]() { pollGroup(HIGH_FREQUENCY); });

    // Low frequency timer (2 Hz = 500 ms)
    m_pollTimers[LOW_FREQUENCY] = new QTimer(this);
    m_pollTimers[LOW_FREQUENCY]->setInterval(500);
    connect(m_pollTimers[LOW_FREQUENCY], &QTimer::timeout,
            this, [this]() { pollGroup(LOW_FREQUENCY); });
}

void DeltaModbusClient::setupPollingGroups() {
    // High frequency group - analog values (20 Hz)
    m_pollingGroups[HIGH_FREQUENCY]["AD_RPM"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::AD_RPM, 1);
    m_pollingGroups[HIGH_FREQUENCY]["TK_RPM"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::TK_RPM, 2);
    m_pollingGroups[HIGH_FREQUENCY]["ST_RPM"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::ST_RPM, 2);

    // Low frequency group - discrete inputs and command outputs (2 Hz)
    m_pollingGroups[LOW_FREQUENCY]["TK_PERCENT"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::TK_PERCENT, 2);
    m_pollingGroups[LOW_FREQUENCY]["ST_PERCENT"] =
        PolledRegister(QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::ST_PERCENT, 2);
    // Discrete inputs
    m_pollingGroups[LOW_FREQUENCY]["S1"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S1, 1);
    m_pollingGroups[LOW_FREQUENCY]["S2"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S2, 1);
    m_pollingGroups[LOW_FREQUENCY]["S3"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S3, 1);
    m_pollingGroups[LOW_FREQUENCY]["S4"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S4, 1);
    m_pollingGroups[LOW_FREQUENCY]["S5"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S5, 1);
    m_pollingGroups[LOW_FREQUENCY]["S6"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S6, 1);
    m_pollingGroups[LOW_FREQUENCY]["S7"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S7, 1);
    m_pollingGroups[LOW_FREQUENCY]["S8"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S8, 1);
    m_pollingGroups[LOW_FREQUENCY]["S9"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S9, 1);
    m_pollingGroups[LOW_FREQUENCY]["S10"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S10, 1);
    m_pollingGroups[LOW_FREQUENCY]["S11"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S11, 1);
    m_pollingGroups[LOW_FREQUENCY]["S12"] =
        PolledRegister(QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S12, 1);

    // Command outputs
    m_pollingGroups[LOW_FREQUENCY]["K1"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K1, 1);
    m_pollingGroups[LOW_FREQUENCY]["K2"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K2, 1);
    m_pollingGroups[LOW_FREQUENCY]["K3"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K3, 1);
    m_pollingGroups[LOW_FREQUENCY]["K4"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K4, 1);
    m_pollingGroups[LOW_FREQUENCY]["K5"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K5, 1);
    m_pollingGroups[LOW_FREQUENCY]["K6"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::K6, 1);

    // Control and status registers
    m_pollingGroups[LOW_FREQUENCY]["M0_STOP_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS, 1);
    m_pollingGroups[LOW_FREQUENCY]["M11_READY_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS, 1);
    m_pollingGroups[LOW_FREQUENCY]["M12_START_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS, 1);
    m_pollingGroups[LOW_FREQUENCY]["M14_COMPLETE_STATUS"] =
        PolledRegister(QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS, 1);
}

void DeltaModbusClient::pollGroup(PollingGroup group) {
    if (!isConnected()) return;

    auto& registers = m_pollingGroups[group];
    for (auto it = registers.constBegin(); it != registers.constEnd(); ++it) {
        const PolledRegister& reg = it.value();
        m_queue->enqueueRead(reg.type, reg.address, reg.count, it.key());
    }
}


bool DeltaModbusClient::connectToDevice(const QString& address, quint16 port) {
    if (m_client->state() != QModbusDevice::UnconnectedState) {
        disconnectFromDevice();
        QThread::msleep(100);
    }

    m_client->setConnectionParameter(QModbusDevice::NetworkAddressParameter, address);
    m_client->setConnectionParameter(QModbusDevice::NetworkPortParameter, port);
    if (!m_client->connectDevice()) {
        QString error = "Failed to initiate connection: " + m_client->errorString();
        emit errorOccurred(error);
        return false;
    }

    return true;
}

void DeltaModbusClient::disconnectFromDevice() {
    // Останавливаем все таймеры опроса
    for (QTimer* timer : m_pollTimers) {
        if (timer) {
            timer->stop();
        }
    }

    if (m_handler) {
        m_handler->stop();
    }
    if (m_queue) {
        m_queue->clear();
    }

    if (m_client && m_client->state() != QModbusDevice::UnconnectedState) {
        m_client->disconnectDevice();
    }
}

bool DeltaModbusClient::isConnected() const {
    return m_client->state() == QModbusDevice::ConnectedState;
}

void DeltaModbusClient::setLocalPort(quint16 port) {
    if (m_client->state() != QModbusDevice::UnconnectedState) {
        qWarning() << "Cannot set local port while connected. Disconnecting first.";
        disconnectFromDevice();
    }

    if (auto deltaClient = qobject_cast<CustomModbusClient*>(m_client.data())) {
        deltaClient->bindToPort(port);
        m_localPort = port;
        qDebug() << "Local port set to:" << port;
    } else {
        qCritical() << "Failed to cast to CustomModbusClient";
    }
}

// New unified methods
void DeltaModbusClient::readRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 count) {
    if (!isConnected()) {
        emit errorOccurred("Not connected to device");
        return;
    }
    m_queue->enqueueRead(type, address, count);
}

void DeltaModbusClient::writeRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value) {
    if (!isConnected()) {
        QString errorMsg = "Not connected to device";
        emit errorOccurred(errorMsg);
        return;
    }
    m_queue->enqueueWrite(type, address, value);
}

void DeltaModbusClient::removePolledRegister(const QString& name) {
    // Удаляем из всех групп
    for (auto& group : m_pollingGroups) {
        group.remove(name);
    }
}

void DeltaModbusClient::clearPolledRegisters() {
    // Очищаем все группы
    for (auto& group : m_pollingGroups) {
        group.clear();
    }
}

void DeltaModbusClient::addPolledRegisterWithFrequency(const QString& name,
                                                       QModbusDataUnit::RegisterType type,
                                                       quint16 address,
                                                       quint16 count,
                                                       PollingFrequency frequency) {
    PollingGroup group = (frequency == HighFrequency) ? HIGH_FREQUENCY : LOW_FREQUENCY;
    m_pollingGroups[group][name] = PolledRegister(type, address, count);

    // qDebug() << "Added parameter to polling group:" << name
    //          << "address:" << QString::number(address, 16)
    //          << "frequency:" << (frequency == HighFrequency ? "20Hz" : "2Hz");
}

void DeltaModbusClient::setOperationMode(const QString& mode) {
    if (m_currentMode != mode) {
        m_currentMode = mode;
        // adjustPollingForMode(mode);
    }
}

void DeltaModbusClient::onStateChanged(QModbusDevice::State state) {
    switch (state) {
    case QModbusDevice::ConnectedState:
        if (m_handler) {
            m_handler->start();
        }
        // Запускаем все таймеры опроса
        for (QTimer* timer : m_pollTimers) {
            timer->start();
        }
        initializeD0();
        emit connected();
        break;
    case QModbusDevice::UnconnectedState:
        // Останавливаем все таймеры опроса
        for (QTimer* timer : m_pollTimers) {
            timer->stop();
        }
        if (m_handler) {
            m_handler->stop();
        }
        if (m_queue) {
            m_queue->clear();
        }
        emit disconnected();
        break;
    default:
        break;
    }
}

void DeltaModbusClient::onErrorOccurred(QModbusDevice::Error error) {
    if (error != QModbusDevice::NoError) {
        QString errorMsg = m_client->errorString();
        qWarning() << "Modbus error:" << errorMsg;
        emit errorOccurred(errorMsg);

        if (error == QModbusDevice::TimeoutError &&
            m_client->state() != QModbusDevice::UnconnectedState) {
            m_client->disconnectDevice();
        }
    }
}

void DeltaModbusClient::writeAndVerifyRegister(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, int timeoutMs) {
    if (!isConnected()) {
        emit errorOccurred("Not connected to device");
        return;
    }

    // Сначала записываем значение (оно автоматически попадет в приоритетную очередь)
    writeRegister(type, address, value);

    // Затем добавляем запрос на проверку в ПРИОРИТЕТНУЮ очередь
    VerificationRequest request;
    request.type = type;
    request.address = address;
    request.expectedValue = value;
    request.timestamp = QDateTime::currentDateTime();

    m_verificationRequests[address] = request;

    // Немедленно ставим приоритетное чтение для проверки
    m_queue->enqueuePriorityRead(type, address, 1, "verification");

    // Запускаем таймер проверки
    if (!m_verificationTimer->isActive()) {
        m_verificationTimer->start();
    }

    // Устанавливаем таймаут для проверки
    QTimer::singleShot(timeoutMs, this, [this, address]() {
        if (m_verificationRequests.contains(address)) {
            m_verificationRequests.remove(address);
            emit registerWriteVerified(QModbusDataUnit::HoldingRegisters, address, 0, false);
        }
    });
}

void DeltaModbusClient::onVerificationTimeout() {
    if (m_verificationRequests.isEmpty()) {
        m_verificationTimer->stop();
        return;
    }

    // Для оставшихся запросов на проверку ставим приоритетные чтения
    for (auto it = m_verificationRequests.begin(); it != m_verificationRequests.end(); ++it) {
        quint16 address = it.key();
        m_queue->enqueuePriorityRead(it.value().type, address, 1, "verification");
    }
}

void DeltaModbusClient::onReadCompleted(QModbusDataUnit::RegisterType type, quint16 address, quint16 value, const QString& paramName) {
    emit registerReadCompleted(type, address, value);

    if (!paramName.isEmpty()) {
        emit dataRead(paramName, value);
    }

    // Проверяем, не это ли запрос на верификацию
    if (paramName == "verification" && m_verificationRequests.contains(address)) {
        VerificationRequest request = m_verificationRequests[address];
        bool success = (value == request.expectedValue);
        m_verificationRequests.remove(address);
        emit registerWriteVerified(type, address, value, success);
    }
}

void DeltaModbusClient::onReadsCompleted(QModbusDataUnit::RegisterType type, quint16 address, const QVector<quint16>& values) {
    emit registersReadCompleted(type, address, values);
}

void DeltaModbusClient::onWriteCompleted(QModbusDataUnit::RegisterType type, quint16 address, bool success) {
    emit registerWriteCompleted(type, address, success);
    // Если запись не удалась, удаляем соответствующий запрос на верификацию
    if (!success && m_verificationRequests.contains(address)) {
        m_verificationRequests.remove(address);
        emit registerWriteVerified(type, address, 0, false);
    }
}

void DeltaModbusClient::onRequestFailed(const QString& error) {
    emit errorOccurred(error);
}

void DeltaModbusClient::initializeD0() {
    if (isConnected() && !m_initialD0Written) {
        writeRegister(QModbusDataUnit::HoldingRegisters, 0x0000, 0); // D0 = 0
        m_initialD0Written = true;
    }
}

void DeltaModbusClient::setConnectionParameter(QModbusDevice::ConnectionParameter param, const QVariant& value) {
    if (m_client) {
        m_client->setConnectionParameter(param, value);
    }
}
