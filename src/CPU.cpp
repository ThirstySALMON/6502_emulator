#include "CPU.h"

void CPU::setflag(FLAGS f, bool value) {
    if (value) status |=  f;
    else       status &= ~f;
}

bool CPU::getflag(FLAGS f) const {
    return status & f;
}

void CPU::reset(Memory& mem) {
    Cycles = 7;

    SP = 0xFF;
    A = X = Y = 0;
    status = 0x00;
    setflag(I, true);
    setflag(U, true);

    Cycles--; // cycle 1
    Cycles--; // cycle 2

    mem.ReadByte(0x0100 + SP--); Cycles--; // cycle 3
    mem.ReadByte(0x0100 + SP--); Cycles--; // cycle 4
    mem.ReadByte(0x0100 + SP--); Cycles--; // cycle 5

    Byte lo = mem.ReadByte(0xFFFC); Cycles--; // cycle 6
    Byte hi = mem.ReadByte(0xFFFD); Cycles--; // cycle 7

    PC = (hi << 8) | lo;
}

void CPU::clock(Memory& mem , int32_t cycles) {
    // next step — fetch, decode, execute
}