#ifndef INC_6502_EMULATOR_CPU_H
#define INC_6502_EMULATOR_CPU_H

# include "incl.h"



class CPU {
private:

    Byte  A      = 0x00;  // Accumulator
    Byte  X      = 0x00;  // Index X
    Byte  Y      = 0x00;  // Index Y
    Byte  SP     = 0xFD;  // Stack pointer
    Word PC     = 0x0000;// Program counter
    Byte  status = 0x00;  // Processor status flags (P register)

    // Status flag bitmasks
    enum FLAGS : uint8_t {
        C = (1 << 0),  // Carry
        Z = (1 << 1),  // Zero
        I = (1 << 2),  // Interrupt disable
        D = (1 << 3),  // Decimal (ignored on NES)
        B = (1 << 4),  // Break
        U = (1 << 5),  // Unused — always 1
        V = (1 << 6),  // Overflow
        N = (1 << 7),  // Negative
    };
    void reset();

    void setflag(FLAGS f);
    void getflag(FLAGS f) const;
};

#endif //INC_6502_EMULATOR_CPU_H