#include "CPU.h"



CPU::CPU() {
    // Initialize all opcodes to XXX (illegal) by default
    for (auto& inst : lookup) {
        inst = {"XXX", &CPU::XXX, &CPU::IMP, 2};
    }

    // Define valid opcodes
    // --- LDA ---
    lookup[0xA9] = {"LDA_imm", &CPU::LDA, &CPU::IMM, 2};
    lookup[0xA5] = {"LDA_zp",  &CPU::LDA, &CPU::ZP0, 3};
    lookup[0xB5] = {"LDA_zpx", &CPU::LDA, &CPU::ZPX, 4};
    lookup[0xAD] = {"LDA_abs", &CPU::LDA, &CPU::ABS, 4};
    lookup[0xBD] = {"LDA_abx", &CPU::LDA, &CPU::ABX, 4};
    lookup[0xB9] = {"LDA_aby", &CPU::LDA, &CPU::ABY, 4};
    lookup[0xA1] = {"LDA_izx", &CPU::LDA, &CPU::IZX, 6};
    lookup[0xB1] = {"LDA_izy", &CPU::LDA, &CPU::IZY, 5};

    // --- STA ---
    lookup[0x85] = {"STA_zp",  &CPU::STA, &CPU::ZP0, 3};
    lookup[0x95] = {"STA_zpx", &CPU::STA, &CPU::ZPX, 4};
    lookup[0x8D] = {"STA_abs", &CPU::STA, &CPU::ABS, 4};
    lookup[0x9D] = {"STA_abx", &CPU::STA, &CPU::ABX, 5};  // note: always 5, no page penalty
    lookup[0x99] = {"STA_aby", &CPU::STA, &CPU::ABY, 5};  // note: always 5, no page penalty
    lookup[0x81] = {"STA_izx", &CPU::STA, &CPU::IZX, 6};
    lookup[0x91] = {"STA_izy", &CPU::STA, &CPU::IZY, 6};  // note: always 6, no page penalty

    // --- LDX ---
    lookup[0xA2] = {"LDX_imm", &CPU::LDX, &CPU::IMM, 2};
    lookup[0xA6] = {"LDX_zp",  &CPU::LDX, &CPU::ZP0, 3};
    lookup[0xB6] = {"LDX_zpy", &CPU::LDX, &CPU::ZPY, 4};
    lookup[0xAE] = {"LDX_abs", &CPU::LDX, &CPU::ABS, 4};
    lookup[0xBE] = {"LDX_aby", &CPU::LDX, &CPU::ABY, 4};  // +1 on page cross

    // --- STX ---
    lookup[0x86] = {"STX_zp",  &CPU::STX, &CPU::ZP0, 3};
    lookup[0x96] = {"STX_zpy", &CPU::STX, &CPU::ZPY, 4};
    lookup[0x8E] = {"STX_abs", &CPU::STX, &CPU::ABS, 4};

    // --- LDY ---
    lookup[0xA0] = {"LDY_imm", &CPU::LDY, &CPU::IMM, 2};
    lookup[0xA4] = {"LDY_zp",  &CPU::LDY, &CPU::ZP0, 3};
    lookup[0xB4] = {"LDY_zpx", &CPU::LDY, &CPU::ZPX, 4};
    lookup[0xAC] = {"LDY_abs", &CPU::LDY, &CPU::ABS, 4};
    lookup[0xBC] = {"LDY_abx", &CPU::LDY, &CPU::ABX, 4};  // +1 on page cross

    // --- STY ---
    lookup[0x84] = {"STY_zp",  &CPU::STY, &CPU::ZP0, 3};
    lookup[0x94] = {"STY_zpx", &CPU::STY, &CPU::ZPX, 4};
    lookup[0x8C] = {"STY_abs", &CPU::STY, &CPU::ABS, 4};



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

uint8_t CPU::IMM() {
    AbsAddr = PC++;
    return 0;
}

uint8_t CPU::ZP0() {
    AbsAddr = (0x00 << 8) | this->mem->ReadByte(PC++);
    return 0;
}

uint8_t CPU::ZPX() {
    Byte lo = (this->mem->ReadByte(PC++) + this->X) & 0xFF;
    AbsAddr = (0x00 << 8) | lo;
    return 0;
}

uint8_t CPU::ZPY() {
    Byte lo = (this->mem->ReadByte(PC++) + this->Y) & 0xFF;;
    AbsAddr = (0x00 << 8) | lo;
    return 0;
}

uint8_t CPU::ABS() {
    Byte lo = this->mem->ReadByte(PC++);
    Byte hi = this->mem->ReadByte(PC++);
    AbsAddr = (hi << 8) | lo;
    return 0;
}

uint8_t CPU::ABX() { // Can take an extra cycle
    Byte lo = this->mem->ReadByte(PC++);
    Byte hi = this->mem->ReadByte(PC++);
    Word Base = (hi << 8) | lo;
    AbsAddr = Base + X;
    if ((AbsAddr & 0xFF00) != (Base & 0xFF00)) {
        return 1;
    }
    return 0;
}

uint8_t CPU::ABY() {
    Byte lo = this->mem->ReadByte(PC++);
    Byte hi = this->mem->ReadByte(PC++);
    Word Base = (hi << 8) | lo;
    AbsAddr = Base + Y;
    if ((AbsAddr & 0xFF00) != (Base & 0xFF00)) return 1;
    return 0;
}

uint8_t CPU::IND() {

    Byte lo = this->mem->ReadByte(PC++);
    Byte hi = this->mem->ReadByte(PC++);
    Byte finaldst_lo = this->mem->ReadByte((hi << 8) | lo); // lower part

    Byte finaldst_hi;
    if (lo == 0xFF) {
       finaldst_hi = this->mem->ReadByte((hi << 8) | 0x00); // higher part
    }
    else {
        finaldst_hi = this->mem->ReadByte(((hi << 8) | lo) +1 ); // higher part
    }

    AbsAddr = (finaldst_hi << 8) | finaldst_lo ;

    return 0;
}

uint8_t CPU::IZX() {

    Byte zp = this->mem->ReadByte(PC++);
    Byte ptr = (zp + X) & 0xFF;           // add X, wrap within zero page

    Byte lo  = mem->ReadByte(ptr);
    Byte hi  = mem->ReadByte((ptr + 1) & 0xFF);  // wrap high byte ptr too

    AbsAddr  = (hi << 8) | lo;
    return 0;
}


uint8_t CPU::IZY() {
    Byte ptr  = this->mem->ReadByte(PC++);
    Byte lo   = mem->ReadByte(ptr & 0xFF);
    Byte hi   = mem->ReadByte((ptr + 1) & 0xFF);
    Word base = (hi << 8) | lo;
    AbsAddr   = base + Y;
    if ((AbsAddr & 0xFF00) != (base & 0xFF00)) return 1;
    return 0;
}

uint8_t CPU::REL() { // needs testing
    int8_t offset = static_cast<int8_t>(this->mem->ReadByte(PC++));
    AbsAddr = PC + offset;
    return 0;
}

uint8_t CPU::IMP() {
    fetchedByte = A;
    return 0;
}









// Instruction implementation


uint8_t CPU::LDA() {
    Fetch_Byte();
    A = fetchedByte;
    setflag(Z, A == 0x00);
    setflag(N, A & 0x80);
    return 1;
}

uint8_t CPU::STA() {
    mem->WriteByte(AbsAddr , A);
    return 0;
}

uint8_t CPU::LDX() {
    Fetch_Byte();
    X = fetchedByte;
    setflag(Z, X == 0x00);
    setflag(N, X & 0x80);
    return 1;
}

uint8_t CPU::STX() {
    mem->WriteByte(AbsAddr , X);
    return 0;
}

uint8_t CPU::LDY() {
    Fetch_Byte();
    Y = fetchedByte;
    setflag(Z, Y == 0x00);
    setflag(N, Y & 0x80);
    return 1;
}

uint8_t CPU::STY() {
    mem->WriteByte(AbsAddr , Y);
    return 0;
}
uint8_t CPU::JMP() { return 0; }
uint8_t CPU::JSR() { return 0; }
uint8_t CPU::RTS() { return 0; }
uint8_t CPU::AND() { return 0; }
uint8_t CPU::ORA() { return 0; }
uint8_t CPU::EOR() { return 0; }
uint8_t CPU::ADC() { return 0; }
uint8_t CPU::SBC() { return 0; }
uint8_t CPU::INC() { return 0; }
uint8_t CPU::DEC() { return 0; }
uint8_t CPU::INX() { return 0; }
uint8_t CPU::DEX() { return 0; }
uint8_t CPU::INY() { return 0; }
uint8_t CPU::DEY() { return 0; }
uint8_t CPU::CMP() { return 0; }
uint8_t CPU::CPX() { return 0; }
uint8_t CPU::CPY() { return 0; }
uint8_t CPU::BEQ() { return 0; }
uint8_t CPU::BNE() { return 0; }
uint8_t CPU::BCS() { return 0; }
uint8_t CPU::BCC() { return 0; }
uint8_t CPU::BMI() { return 0; }
uint8_t CPU::BPL() { return 0; }
uint8_t CPU::BVS() { return 0; }
uint8_t CPU::BVC() { return 0; }
uint8_t CPU::TAX() { return 0; }
uint8_t CPU::TXA() { return 0; }
uint8_t CPU::TAY() { return 0; }
uint8_t CPU::TYA() { return 0; }
uint8_t CPU::TXS() { return 0; }
uint8_t CPU::TSX() { return 0; }
uint8_t CPU::PHA() { return 0; }
uint8_t CPU::PLA() { return 0; }
uint8_t CPU::PHP() { return 0; }
uint8_t CPU::PLP() { return 0; }
uint8_t CPU::SEC() { return 0; }
uint8_t CPU::CLC() { return 0; }
uint8_t CPU::SEI() { return 0; }
uint8_t CPU::CLI() { return 0; }
uint8_t CPU::SED() { return 0; }
uint8_t CPU::CLD() { return 0; }
uint8_t CPU::CLV() { return 0; }
uint8_t CPU::NOP() { return 0; }
uint8_t CPU::XXX() { return 0; }

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

        uint8_t extraAddr = (this->*inst.addrmode)();
        uint8_t extraOp   = (this->*inst.operate)();
        cycles -= inst.cycles + (extraAddr & extraOp);
    }
}