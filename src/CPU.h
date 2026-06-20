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
    int64_t Cycles = 0;






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

    CPU();
    void reset(Memory& mem);
    void clock(Memory& mem , int32_t);
    uint64_t get_elapsed_cycles() const; // returns total cycles used
    void setflag(FLAGS f, bool value);
    bool getflag(FLAGS f) const;

private:
    // helpers
    Memory* mem = nullptr; // pointer to memory
    Word AbsAddr = 0x0000; // effective address after address resolution
    Byte fetchedByte = 0x00;   // fetched operand value for operations
    Word fetchedWord = 0x0000; // fetched operand value (Word)

    uint64_t Total_cycles = 0 ; // total cycles consumed since last reset
    struct Instruction {
        const char*        name;
        uint8_t (CPU::*operate)();
        uint8_t (CPU::*addrmode)();
        uint8_t            cycles;
    };



    Byte Fetch_inst();  // fetches the byte at memory address PC and increments PC by 1
    Byte Fetch_Byte();  // fetches data using AbsAddr (or returns fetched for IMM mode)
    Word Fetch_Word(); //  fetches data using AbsAddr (Word)


    Instruction DecodeInst (Byte);





    std::array<Instruction, 256> lookup; // table for the 256 opcodes of the 6502

    // Addressing modes
    uint8_t IMM();  // Immediate
    uint8_t ZP0();  // Zero page
    uint8_t ZPX();  // Zero page, X
    uint8_t ZPY();  // Zero page, Y
    uint8_t ABS();  // Absolute
    uint8_t ABX();  // Absolute, X
    uint8_t ABY();  // Absolute, Y
    uint8_t IND();  // Indirect
    uint8_t IZX();  // Indirect, X
    uint8_t IZY();  // Indirect, Y
    uint8_t REL();  // Relative  (branches)
    uint8_t IMP();  // Implied   (no operand)

    // Operations
    uint8_t LDA(); uint8_t STA();
    uint8_t LDX(); uint8_t STX();
    uint8_t LDY(); uint8_t STY();
    uint8_t JMP(); uint8_t JSR(); uint8_t RTS();
    uint8_t AND(); uint8_t ORA(); uint8_t EOR();
    uint8_t ADC(); uint8_t SBC();
    uint8_t INC(); uint8_t DEC();
    uint8_t INX(); uint8_t DEX();
    uint8_t INY(); uint8_t DEY();
    uint8_t CMP(); uint8_t CPX(); uint8_t CPY();
    uint8_t BEQ(); uint8_t BNE();
    uint8_t BCS(); uint8_t BCC();
    uint8_t BMI(); uint8_t BPL();
    uint8_t BVS(); uint8_t BVC();
    uint8_t TAX(); uint8_t TXA();
    uint8_t TAY(); uint8_t TYA();
    uint8_t TXS(); uint8_t TSX();
    uint8_t PHA(); uint8_t PLA();
    uint8_t PHP(); uint8_t PLP();
    uint8_t SEC(); uint8_t CLC();
    uint8_t SEI(); uint8_t CLI();
    uint8_t SED(); uint8_t CLD();
    uint8_t CLV();
    uint8_t NOP();
    uint8_t XXX();  // illegal/unknown opcode handler


};

#endif //INC_6502_EMULATOR_CPU_H