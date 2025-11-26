#include "PollingConfiguratorFactory.h"
#include "core/mapping/DeltaAddressMap.h"

void PollingConfiguratorFactory::configureDefaultPolling(IModbusClient *client)
{
    if (!client) return;

    // Настройка высокочастотных каналов (20 Гц)
    client->addPolledRegisterWithFrequency("AD_RPM",
                                           QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::AD_RPM, 1,
                                           IModbusClient::HighFrequency);
    client->addPolledRegisterWithFrequency("TK_RPM",
                                           QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::TK_RPM, 2,
                                           IModbusClient::HighFrequency);
    client->addPolledRegisterWithFrequency("ST_RPM",
                                           QModbusDataUnit::HoldingRegisters, DeltaAS332T::Addresses::ST_RPM, 2,
                                           IModbusClient::HighFrequency);

    // Настройка низкочастотных каналов (2 Гц)
    client->addPolledRegisterWithFrequency("S1",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S1, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S2",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S2, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S3",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S3, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S4",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S4, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S5",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S5, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S6",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S6, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S7",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S7, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S8",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S8, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S9",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S9, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S10",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S10, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S11",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S11, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("S12",
                                           QModbusDataUnit::DiscreteInputs, DeltaAS332T::Addresses::S12, 1,
                                           IModbusClient::LowFrequency);

    client->addPolledRegisterWithFrequency("K1",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K1, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K2",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K2, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K3",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K3, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K4",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K4, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K5",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K5, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("K6",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::K6, 1,
                                           IModbusClient::LowFrequency);

    client->addPolledRegisterWithFrequency("M0_STOP_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M0_STOP_STATUS, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("M11_READY_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M11_READY_STATUS, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("M12_START_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M12_START_STATUS, 1,
                                           IModbusClient::LowFrequency);
    client->addPolledRegisterWithFrequency("M14_COMPLETE_STATUS",
                                           QModbusDataUnit::Coils, DeltaAS332T::Addresses::M14_COMPLETE_STATUS, 1,
                                           IModbusClient::LowFrequency);
}
