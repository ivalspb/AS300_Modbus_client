#pragma once
#include "core/interfaces/IModbusClient.h"
#include "core/interfaces/IPollingConfigurator.h"

class PollingConfiguratorFactory {
public:
    static void configureDefaultPolling(IModbusClient* client);
};
