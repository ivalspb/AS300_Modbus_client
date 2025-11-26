#pragma once
#include <QObject>

class IPollingConfigurator {
public:
    virtual ~IPollingConfigurator() = default;

    virtual void setHighFrequencyInterval(int ms) = 0;
    virtual void setLowFrequencyInterval(int ms) = 0;
    virtual void addToHighFrequencyGroup(const QString& name) = 0;
    virtual void addToLowFrequencyGroup(const QString& name) = 0;
    virtual void removeFromPolling(const QString& name) = 0;
};
