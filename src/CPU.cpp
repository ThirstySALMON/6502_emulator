#include "CPU.h"



CPU::CPU() {
    // Initialize all opcodes to XXX (illegal) by default
    for (auto& inst : lookup) {
        inst = {"XXX", &CPU::XXX, &CPU::IMP, 2};
    }

    // Define valid opcodes
    lookup[0xA9] = {"LDA_imm", &CPU::LDA, &CPU::IMM, 2};
    lookup[0xA6] = {"LDA_zp", &CPU::LDA, &CPU::ZP0, 3};
    lookup[0xAD] = {"LDA_a", &CPU::LDA, &CPU::ABS, 4};
    lookup[0xEA] = {"NOP", &CPU::NOP, &CPU::IMP, 2};
}
void CPU::setflag(FLAGS f, bool value) {
    if (value) status |=  f;
    else       status &= ~f;
}

bool CPU::getflag(FLAGS f) const {
    return status & f;
}

Byte CPU::Fetch_inst() {
    return mem->ReadByte(PC++);
}

Byte CPU::Fetch_Byte() {
    // For implied/accumulator modes, fetched is already set
    // For other modes, read from the resolved address
    fetchedByte =  mem->ReadByte(AbsAddr);
    return fetchedByte;
}

CPU::Instruction CPU::DecodeInst(Byte opcode) {
    return lookup[opcode];
}

void CPU::IMM() {
    // Immediate mode: the operand is the next byte after the opcode
    AbsAddr = PC++;
}

void CPU::ZP0() {
    AbsAddr = (0x00 << 8) | this->mem->ReadByte(PC++);
}

void CPU::ZPX() {
}

void CPU::ZPY() {
}

void CPU::ABS() {
    Byte lo = this->mem->ReadByte(PC++);
    Byte hi = this->mem->ReadByte(PC++);
    AbsAddr = (hi << 8) | lo;
}

void CPU::ABX() {
}

void CPU::ABY() {
}

void CPU::IND() {
}

void CPU::IZX() {
}

void CPU::IZY() {
}

void CPU::REL() {
}

void CPU::IMP() {
    // Implied mode: no operand, instruction operates on registers
    fetchedByte = A;  // Some implied instructions operate on accumulator
}

void CPU::LDA() {
    // Load Accumulator: A = M
    Fetch_Byte();
    A = fetchedByte;
    setflag(Z, A == 0x00);
    setflag(N, A & 0x80);
}

void CPU::STA() {
}

void CPU::LDX() {
    Fetch_Byte();
    X = fetchedByte;
    setflag(Z, X == 0x00);
    setflag(N, X & 0x80);
}

void CPU::STX() {
}

void CPU::LDY() {
}

void CPU::STY() {
}

void CPU::JMP() {
}

void CPU::JSR() {
}

void CPU::RTS() {
}

void CPU::AND() {
}

void CPU::ORA() {
}

void CPU::EOR() {
}

void CPU::ADC() {
}

void CPU::SBC() {
}

void CPU::INC() {
}

void CPU::DEC() {
}

void CPU::INX() {
}

void CPU::DEX() {
}

void CPU::INY() {
}

void CPU::DEY() {
}

void CPU::CMP() {
}

void CPU::CPX() {
}

void CPU::CPY() {
}

void CPU::BEQ() {
}

void CPU::BNE() {
}

void CPU::BCS() {
}

void CPU::BCC() {
}

void CPU::BMI() {
}

void CPU::BPL() {
}

void CPU::BVS() {
}

void CPU::BVC() {
}

void CPU::TAX() {
}

void CPU::TXA() {
}

void CPU::TAY() {
}

void CPU::TYA() {
}

void CPU::TXS() {
}

void CPU::TSX() {
}

void CPU::PHA() {
}

void CPU::PLA() {
}

void CPU::PHP() {
}

void CPU::PLP() {
}

void CPU::SEC() {
}

void CPU::CLC() {
}

void CPU::SEI() {
}

void CPU::CLI() {
}

void CPU::SED() {
}

void CPU::CLD() {
}

void CPU::CLV() {
}

void CPU::NOP() {
    // No operation - does nothing
}

void CPU::XXX() {
    // Illegal/unknown opcode - acts as NOP
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

void CPU::clock(Memory& mem, int32_t cycles) {
    this->mem = &mem;

    while (cycles > 0) {
        Byte opcode = Fetch_inst();

        Instruction inst = DecodeInst(opcode);


        if (inst.addrmode == nullptr || inst.operate == nullptr) {
            // Handle as illegal opcode - treat like NOP but consume 1 cycle
            cycles--;
            continue;
        }

        // Execute addressing mode (sets up AbsAddr and/or fetched)
        (this->*inst.addrmode)();

        // Execute the operation
        (this->*inst.operate)();

        // Consume cycles
        cycles -= inst.cycles;
    }
}