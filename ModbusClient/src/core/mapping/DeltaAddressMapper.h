#pragma once
#include "core/interfaces/IAddressMapper.h"
#include "DeltaController.h"
#include <QMap>

class DeltaAddressMapper : public IAddressMapper {
public:
    DeltaAddressMapper();
    ~DeltaAddressMapper() override = default;

    quint16 getDiscreteInputAddress(int input) const override;
    quint16 getCommandOutputAddress(int output) const override;
    quint16 getAnalogAddress(int analog) const override;
    QString getDiscreteInputName(int input) const override;
    QString getCommandOutputName(int output) const override;
    QString getAnalogName(int analog) const override;
    bool isValidAddress(quint16 address) const override;

private:
    void initializeMappings();

    QMap<int, quint16> m_discreteInputMap;
    QMap<int, quint16> m_commandOutputMap;
    QMap<int, quint16> m_analogMap;
    QMap<int, QString> m_discreteInputNames;
    QMap<int, QString> m_commandOutputNames;
    QMap<int, QString> m_analogNames;
};

