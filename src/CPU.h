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




    CPU();

    void reset(Memory& mem);
    void clock(Memory& mem , int32_t);

    void setflag(FLAGS f, bool value);
    bool getflag(FLAGS f) const;

private:
    // helpers
    Memory* mem = nullptr; // pointer to memory
    Word AbsAddr = 0x0000; // effective address after address resolution
    Byte fetchedByte = 0x00;   // fetched operand value for operations
    Word fetchedWord = 0x0000; // fetched operand value (Word)
    struct Instruction {
        const char*      name; // inst name
        void (CPU::*operate)(); // operation
        void (CPU::*addrmode)(); // addressing mode
        uint8_t          cycles; // cycles taken
    };

    Byte Fetch_inst();  // fetches the byte at memory address PC and increments PC by 1
    Byte Fetch_Byte();      // fetches data using AbsAddr (or returns fetched for IMM mode)
    Word Fetch_Word(); //  fetches data using AbsAddr (Word)


    Instruction DecodeInst (Byte);





    std::array<Instruction, 256> lookup; // table for the 256 opcodes of the 6502

    // Addressing modes
    void IMM();  // Immediate
    void ZP0();  // Zero page
    void ZPX();  // Zero page, X
    void ZPY();  // Zero page, Y
    void ABS();  // Absolute
    void ABX();  // Absolute, X
    void ABY();  // Absolute, Y
    void IND();  // Indirect
    void IZX();  // Indirect, X
    void IZY();  // Indirect, Y
    void REL();  // Relative  (branches)
    void IMP();  // Implied   (no operand)

    // Operations
    void LDA(); void STA();
    void LDX(); void STX();
    void LDY(); void STY();
    void JMP(); void JSR(); void RTS();
    void AND(); void ORA(); void EOR();
    void ADC(); void SBC();
    void INC(); void DEC();
    void INX(); void DEX();
    void INY(); void DEY();
    void CMP(); void CPX(); void CPY();
    void BEQ(); void BNE();
    void BCS(); void BCC();
    void BMI(); void BPL();
    void BVS(); void BVC();
    void TAX(); void TXA();
    void TAY(); void TYA();
    void TXS(); void TSX();
    void PHA(); void PLA();
    void PHP(); void PLP();
    void SEC(); void CLC();
    void SEI(); void CLI();
    void SED(); void CLD();
    void CLV();
    void NOP();
    void XXX();  // illegal/unknown opcode handler


};

#endif //INC_6502_EMULATOR_CPU_H