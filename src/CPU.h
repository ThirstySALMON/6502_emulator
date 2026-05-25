#ifndef INC_6502_EMULATOR_CPU_H
#define INC_6502_EMULATOR_CPU_H

#include "incl.h"
#include "Memory.h"

class CPU {
public:
    Byte    A      = 0x00;
    Byte    X      = 0x00;
    Byte    Y      = 0x00;
    Byte    SP     = 0xFF;  // 0xFF before reset walks it to 0xFD
    Word    PC     = 0x0000;
    Byte    status = 0x00;
    int32_t Cycles = 0;

    enum FLAGS : uint8_t {
        C = (1 << 0),
        Z = (1 << 1),
        I = (1 << 2),
        D = (1 << 3),
        B = (1 << 4),
        U = (1 << 5),
        V = (1 << 6),
        N = (1 << 7),
    };

    CPU() = default;

    void reset(Memory& mem);
    void clock(Memory& mem , int32_t);

    void setflag(FLAGS f, bool value);
    bool getflag(FLAGS f) const;
};

#endif //INC_6502_EMULATOR_CPU_H