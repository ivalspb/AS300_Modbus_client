#pragma once
#include <QString>
#include <QMap>

    class IAddressMapper {
public:
    virtual ~IAddressMapper() = default;

    virtual quint16 getDiscreteInputAddress(int input) const = 0;
    virtual quint16 getCommandOutputAddress(int output) const = 0;
    virtual quint16 getAnalogAddress(int analog) const = 0;
    virtual QString getDiscreteInputName(int input) const = 0;
    virtual QString getCommandOutputName(int output) const = 0;
    virtual QString getAnalogName(int analog) const = 0;
    virtual bool isValidAddress(quint16 address) const = 0;
};

