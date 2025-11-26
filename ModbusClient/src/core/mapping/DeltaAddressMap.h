#pragma once
#include <QtGlobal>
#include <QVector>

namespace DeltaAS332T {
    namespace Addresses {
        // Discrete Inputs (X0.0 - X0.11) - Сигналы S1-S12
        const quint16 S1 = 0x6000;   // X0.0 Откл БУТС
        const quint16 S2 = 0x6001;   // X0.1 ОРТС
        const quint16 S3 = 0x6002;   // X0.2 ЭМЗС
        const quint16 S4 = 0x6003;   // X0.3 АЗТС
        const quint16 S5 = 0x6004;   // X0.4 ЭСТС
        const quint16 S6 = 0x6005;   // X0.5 Откл ЭСТС (СОЭС)
        const quint16 S7 = 0x6006;   // X0.6 ИП АЗТС Вкл.
        const quint16 S8 = 0x6007;   // X0.7 ИП ЭСТС Вкл.
        const quint16 S9 = 0x6008;   // X0.8 1й канал АЗТС Вкл.
        const quint16 S10 = 0x6009;  // X0.9 2й канал АЗТС Вкл.
        const quint16 S11 = 0x600A;  // X0.10 ЭСТС Вкл.
        const quint16 S12 = 0x600B;  // X0.11 ПЧ готов

        // Command Outputs (Y0.0 - Y0.8) - Команды K1-K6
        const quint16 K1 = 0xA000;   // Y0.0 Пуск ТС
        const quint16 K2 = 0xA001;   // Y0.1 Стоп ТС
        const quint16 K3 = 0xA002;   // Y0.2 СТОП-кран
        const quint16 K4 = 0xA003;   // Y0.3 Режим Консервации
        const quint16 K5 = 0xA004;   // Y0.4 Режим Холодной прокрутки
        const quint16 K6 = 0xA008;   // Y0.8 Активация Выходов ПЧ

        // Analog Values - регистры D (D0-D29999 = 0x0000-0x752F)
        const quint16 AD_RPM = 0x000A;      // D10 Частота вращения АД (об/мин)
        const quint16 TK_RPM = 0x0015;      // D21 (DWORD - читать 2 регистра) Частота вращения ТК (об/мин)
        const quint16 TK_PERCENT = 0x0017;  // D23 (DWORD - читать 2 регистра) Частота вращения ТК (%)
        const quint16 ST_RPM = 0x0029;      // D41 (DWORD - читать 2 регистра) Частота вращения СТ (об/мин)
        const quint16 ST_PERCENT = 0x002B;  // D43 (DWORD - читать 2 регистра) Частота вращения СТ (%)

        const quint16 SM_TEST = 0x404C; // SM76 bit флаг запроса отправки данных с Card1

        // / Control registers according to algorithm
        const quint16 D0_MODE_REGISTER = 0x0000;  // D0 - режим работы

        // M-registers for control buttons
        const quint16 M1_READY_CHECK = 0x0001;    // M1 - Проверка готовности
        const quint16 M2_START = 0x0002;          // M2 - ПУСК
        const quint16 M3_STOP = 0x0003;           // M3 - СТОП
        const quint16 M4_RESTART = 0x0004;        // M4 - Повторение запуска
        const quint16 M5_INTERRUPT = 0x0005;      // M5 - Прерывание запуска
        const quint16 M6_EXIT = 0x0006;           // M6 - ВЫХОД

        // Status registers
        const quint16 M0_STOP_STATUS = 0x0000;  // M0 - отсутсвие вращения
        const quint16 M11_READY_STATUS = 0x000B;  // M11 - статус готовности
        const quint16 M12_START_STATUS = 0x000C;  // M12 - статус запуска
        const quint16 M14_COMPLETE_STATUS = 0x000E; // M14 - завершение запуска
    }

    // Value conversion functions
    inline double scaleAnalogValue(quint16 raw, double factor = 1.0) {
        return raw * factor;
    }

    inline double scaleRPM(quint16 raw) {
        return scaleAnalogValue(raw, 1.0); // RPM в об/мин
    }

    inline double scalePercent(quint16 raw) {
        return scaleAnalogValue(raw, 1); // Проценты с коэффициентом 0.1
    }

    // Функции для работы с DWORD значениями (2 регистра)
    inline double readDWORD(const QVector<quint16>& values, int startIndex) {
        if (values.size() >= startIndex + 2) {
            quint32 result = (static_cast<quint32>(values[startIndex + 1]) << 16) | values[startIndex];
            return static_cast<double>(result);
        }
        return 0.0;
    }

    // Функция для записи DWORD значения
    inline QVector<quint16> writeDWORD(quint32 value) {
        QVector<quint16> result;
        result.append(static_cast<quint16>(value & 0xFFFF));
        result.append(static_cast<quint16>((value >> 16) & 0xFFFF));
        return result;
    }
}
