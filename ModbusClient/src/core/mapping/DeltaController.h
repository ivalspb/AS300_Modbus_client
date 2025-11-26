#pragma once

namespace DeltaController {
    enum DiscreteInputs {
        S1 = 0, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12
    };

    enum CommandOutputs {
        K1 = 0, K2, K3, K4, K5, K6
    };

    enum AnalogValues {
        AD_RPM = 0,     // Частота вращения АД (об/мин)
        TK_RPM,         // Частота вращения ТК (об/мин)
        TK_PERCENT,     // Частота вращения ТК (%)
        ST_RPM,         // Частота вращения СТ (об/мин)
        ST_PERCENT      // Частота вращения СТ (%)
    };
}
