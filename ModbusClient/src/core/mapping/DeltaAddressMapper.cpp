#include "DeltaAddressMapper.h"
#include "DeltaAddressMap.h"

DeltaAddressMapper::DeltaAddressMapper() {
    initializeMappings();
}

void DeltaAddressMapper::initializeMappings() {
    // Discrete inputs mapping
    m_discreteInputMap[DeltaController::S1] = DeltaAS332T::Addresses::S1;
    m_discreteInputMap[DeltaController::S2] = DeltaAS332T::Addresses::S2;
    m_discreteInputMap[DeltaController::S3] = DeltaAS332T::Addresses::S3;
    m_discreteInputMap[DeltaController::S4] = DeltaAS332T::Addresses::S4;
    m_discreteInputMap[DeltaController::S5] = DeltaAS332T::Addresses::S5;
    m_discreteInputMap[DeltaController::S6] = DeltaAS332T::Addresses::S6;
    m_discreteInputMap[DeltaController::S7] = DeltaAS332T::Addresses::S7;
    m_discreteInputMap[DeltaController::S8] = DeltaAS332T::Addresses::S8;
    m_discreteInputMap[DeltaController::S9] = DeltaAS332T::Addresses::S9;
    m_discreteInputMap[DeltaController::S10] = DeltaAS332T::Addresses::S10;
    m_discreteInputMap[DeltaController::S11] = DeltaAS332T::Addresses::S11;
    m_discreteInputMap[DeltaController::S12] = DeltaAS332T::Addresses::S12;

    // Names
    m_discreteInputNames[DeltaController::S1] = "S1: Откл БУТС";
    m_discreteInputNames[DeltaController::S2] = "S2: ОРТС";
    m_discreteInputNames[DeltaController::S3] = "S3: ЭМЗС";
    m_discreteInputNames[DeltaController::S4] = "S4: АЗТС";
    m_discreteInputNames[DeltaController::S5] = "S5: ЭСТС";
    m_discreteInputNames[DeltaController::S6] = "S6: Откл ЭСТС (СОЭС)";
    m_discreteInputNames[DeltaController::S7] = "S7: ИП АЗТС Вкл.";
    m_discreteInputNames[DeltaController::S8] = "S8: ИП ЭСТС Вкл.";
    m_discreteInputNames[DeltaController::S9] = "S9: 1й канал АЗТС Вкл.";
    m_discreteInputNames[DeltaController::S10] = "S10: 2й канал АЗТС Вкл.";
    m_discreteInputNames[DeltaController::S11] = "S11: ЭСТС Вкл.";
    m_discreteInputNames[DeltaController::S12] = "S12: ПЧ готов";

    // Command outputs mapping
    m_commandOutputMap[DeltaController::K1] = DeltaAS332T::Addresses::K1;
    m_commandOutputMap[DeltaController::K2] = DeltaAS332T::Addresses::K2;
    m_commandOutputMap[DeltaController::K3] = DeltaAS332T::Addresses::K3;
    m_commandOutputMap[DeltaController::K4] = DeltaAS332T::Addresses::K4;
    m_commandOutputMap[DeltaController::K5] = DeltaAS332T::Addresses::K5;
    m_commandOutputMap[DeltaController::K6] = DeltaAS332T::Addresses::K6;

    m_commandOutputNames[DeltaController::K1] = "Пуск ТС";
    m_commandOutputNames[DeltaController::K2] = "Стоп ТС";
    m_commandOutputNames[DeltaController::K3] = "СТОП-кран";
    m_commandOutputNames[DeltaController::K4] = "Режим Консервации";
    m_commandOutputNames[DeltaController::K5] = "Режим Холодной прокрутки";
    m_commandOutputNames[DeltaController::K6] = "Активация Выходов ПЧ";

    // Analog values mapping
    m_analogMap[DeltaController::AD_RPM] = DeltaAS332T::Addresses::AD_RPM;
    m_analogMap[DeltaController::TK_RPM] = DeltaAS332T::Addresses::TK_RPM;
    m_analogMap[DeltaController::TK_PERCENT] = DeltaAS332T::Addresses::TK_PERCENT;
    m_analogMap[DeltaController::ST_RPM] = DeltaAS332T::Addresses::ST_RPM;
    m_analogMap[DeltaController::ST_PERCENT] = DeltaAS332T::Addresses::ST_PERCENT;

    m_analogNames[DeltaController::AD_RPM] = "Частота вращения АД (об/мин)";
    m_analogNames[DeltaController::TK_RPM] = "Частота вращения ТК (об/мин)";
    m_analogNames[DeltaController::TK_PERCENT] = "Частота вращения ТК (%)";
    m_analogNames[DeltaController::ST_RPM] = "Частота вращения СТ (об/мин)";
    m_analogNames[DeltaController::ST_PERCENT] = "Частота вращения СТ (%)";
}

quint16 DeltaAddressMapper::getDiscreteInputAddress(int input) const {
    return m_discreteInputMap.value(input, 0);
}

quint16 DeltaAddressMapper::getCommandOutputAddress(int output) const {
    return m_commandOutputMap.value(output, 0);
}

quint16 DeltaAddressMapper::getAnalogAddress(int analog) const {
    return m_analogMap.value(analog, 0);
}

QString DeltaAddressMapper::getDiscreteInputName(int input) const {
    return m_discreteInputNames.value(input, "Unknown");
}

QString DeltaAddressMapper::getCommandOutputName(int output) const {
    return m_commandOutputNames.value(output, "Unknown");
}

QString DeltaAddressMapper::getAnalogName(int analog) const {
    return m_analogNames.value(analog, "Unknown");
}

bool DeltaAddressMapper::isValidAddress(quint16 address) const {
    return m_discreteInputMap.values().contains(address) ||
           m_commandOutputMap.values().contains(address) ||
           m_analogMap.values().contains(address);
}
