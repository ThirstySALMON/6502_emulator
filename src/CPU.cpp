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



    // --- Transfers (all IMP, 2 cycles) ---
    lookup[0xAA] = {"TAX", &CPU::TAX, &CPU::IMP, 2};  // A → X, sets N Z
    lookup[0x8A] = {"TXA", &CPU::TXA, &CPU::IMP, 2};  // X → A, sets N Z
    lookup[0xA8] = {"TAY", &CPU::TAY, &CPU::IMP, 2};  // A → Y, sets N Z
    lookup[0x98] = {"TYA", &CPU::TYA, &CPU::IMP, 2};  // Y → A, sets N Z
    lookup[0x9A] = {"TXS", &CPU::TXS, &CPU::IMP, 2};  // X → SP, no flags
    lookup[0xBA] = {"TSX", &CPU::TSX, &CPU::IMP, 2};  // SP → X, sets N Z

    // --- JMP / JSR / RTS ---
    lookup[0x4C] = {"JMP_abs", &CPU::JMP, &CPU::ABS, 3};
    lookup[0x6C] = {"JMP_ind", &CPU::JMP, &CPU::IND, 5};
    lookup[0x20] = {"JSR",     &CPU::JSR, &CPU::ABS, 6};
    lookup[0x60] = {"RTS",     &CPU::RTS, &CPU::IMP, 6};

    lookup[0xEA] = {"NOP", &CPU::NOP, &CPU::IMP, 2};

    // --- AND ---
    lookup[0x29] = {"AND_imm", &CPU::AND, &CPU::IMM, 2};
    lookup[0x25] = {"AND_zp",  &CPU::AND, &CPU::ZP0, 3};
    lookup[0x35] = {"AND_zpx", &CPU::AND, &CPU::ZPX, 4};
    lookup[0x2D] = {"AND_abs", &CPU::AND, &CPU::ABS, 4};
    lookup[0x3D] = {"AND_abx", &CPU::AND, &CPU::ABX, 4};
    lookup[0x39] = {"AND_aby", &CPU::AND, &CPU::ABY, 4};
    lookup[0x21] = {"AND_izx", &CPU::AND, &CPU::IZX, 6};
    lookup[0x31] = {"AND_izy", &CPU::AND, &CPU::IZY, 5};

    // --- ORA ---
    lookup[0x09] = {"ORA_imm", &CPU::ORA, &CPU::IMM, 2};
    lookup[0x05] = {"ORA_zp",  &CPU::ORA, &CPU::ZP0, 3};
    lookup[0x15] = {"ORA_zpx", &CPU::ORA, &CPU::ZPX, 4};
    lookup[0x0D] = {"ORA_abs", &CPU::ORA, &CPU::ABS, 4};
    lookup[0x1D] = {"ORA_abx", &CPU::ORA, &CPU::ABX, 4};
    lookup[0x19] = {"ORA_aby", &CPU::ORA, &CPU::ABY, 4};
    lookup[0x01] = {"ORA_izx", &CPU::ORA, &CPU::IZX, 6};
    lookup[0x11] = {"ORA_izy", &CPU::ORA, &CPU::IZY, 5};

    // --- EOR ---
    lookup[0x49] = {"EOR_imm", &CPU::EOR, &CPU::IMM, 2};
    lookup[0x45] = {"EOR_zp",  &CPU::EOR, &CPU::ZP0, 3};
    lookup[0x55] = {"EOR_zpx", &CPU::EOR, &CPU::ZPX, 4};
    lookup[0x4D] = {"EOR_abs", &CPU::EOR, &CPU::ABS, 4};
    lookup[0x5D] = {"EOR_abx", &CPU::EOR, &CPU::ABX, 4};
    lookup[0x59] = {"EOR_aby", &CPU::EOR, &CPU::ABY, 4};
    lookup[0x41] = {"EOR_izx", &CPU::EOR, &CPU::IZX, 6};
    lookup[0x51] = {"EOR_izy", &CPU::EOR, &CPU::IZY, 5};

    // --- ADC ---
    lookup[0x69] = {"ADC_imm", &CPU::ADC, &CPU::IMM, 2};
    lookup[0x65] = {"ADC_zp",  &CPU::ADC, &CPU::ZP0, 3};
    lookup[0x75] = {"ADC_zpx", &CPU::ADC, &CPU::ZPX, 4};
    lookup[0x6D] = {"ADC_abs", &CPU::ADC, &CPU::ABS, 4};
    lookup[0x7D] = {"ADC_abx", &CPU::ADC, &CPU::ABX, 4};
    lookup[0x79] = {"ADC_aby", &CPU::ADC, &CPU::ABY, 4};
    lookup[0x61] = {"ADC_izx", &CPU::ADC, &CPU::IZX, 6};
    lookup[0x71] = {"ADC_izy", &CPU::ADC, &CPU::IZY, 5};

    // --- SBC ---
    lookup[0xE9] = {"SBC_imm", &CPU::SBC, &CPU::IMM, 2};
    lookup[0xE5] = {"SBC_zp",  &CPU::SBC, &CPU::ZP0, 3};
    lookup[0xF5] = {"SBC_zpx", &CPU::SBC, &CPU::ZPX, 4};
    lookup[0xED] = {"SBC_abs", &CPU::SBC, &CPU::ABS, 4};
    lookup[0xFD] = {"SBC_abx", &CPU::SBC, &CPU::ABX, 4};
    lookup[0xF9] = {"SBC_aby", &CPU::SBC, &CPU::ABY, 4};
    lookup[0xE1] = {"SBC_izx", &CPU::SBC, &CPU::IZX, 6};
    lookup[0xF1] = {"SBC_izy", &CPU::SBC, &CPU::IZY, 5};

    // --- INC ---
    lookup[0xE6] = {"INC_zp",  &CPU::INC, &CPU::ZP0, 5};
    lookup[0xF6] = {"INC_zpx", &CPU::INC, &CPU::ZPX, 6};
    lookup[0xEE] = {"INC_abs", &CPU::INC, &CPU::ABS, 6};
    lookup[0xFE] = {"INC_abx", &CPU::INC, &CPU::ABX, 7};

    // --- DEC ---
    lookup[0xC6] = {"DEC_zp",  &CPU::DEC, &CPU::ZP0, 5};
    lookup[0xD6] = {"DEC_zpx", &CPU::DEC, &CPU::ZPX, 6};
    lookup[0xCE] = {"DEC_abs", &CPU::DEC, &CPU::ABS, 6};
    lookup[0xDE] = {"DEC_abx", &CPU::DEC, &CPU::ABX, 7};

    // --- INX / DEX / INY / DEY ---
    lookup[0xE8] = {"INX", &CPU::INX, &CPU::IMP, 2};
    lookup[0xCA] = {"DEX", &CPU::DEX, &CPU::IMP, 2};
    lookup[0xC8] = {"INY", &CPU::INY, &CPU::IMP, 2};
    lookup[0x88] = {"DEY", &CPU::DEY, &CPU::IMP, 2};

    // --- CMP ---
    lookup[0xC9] = {"CMP_imm", &CPU::CMP, &CPU::IMM, 2};
    lookup[0xC5] = {"CMP_zp",  &CPU::CMP, &CPU::ZP0, 3};
    lookup[0xD5] = {"CMP_zpx", &CPU::CMP, &CPU::ZPX, 4};
    lookup[0xCD] = {"CMP_abs", &CPU::CMP, &CPU::ABS, 4};
    lookup[0xDD] = {"CMP_abx", &CPU::CMP, &CPU::ABX, 4};
    lookup[0xD9] = {"CMP_aby", &CPU::CMP, &CPU::ABY, 4};
    lookup[0xC1] = {"CMP_izx", &CPU::CMP, &CPU::IZX, 6};
    lookup[0xD1] = {"CMP_izy", &CPU::CMP, &CPU::IZY, 5};

    // --- CPX ---
    lookup[0xE0] = {"CPX_imm", &CPU::CPX, &CPU::IMM, 2};
    lookup[0xE4] = {"CPX_zp",  &CPU::CPX, &CPU::ZP0, 3};
    lookup[0xEC] = {"CPX_abs", &CPU::CPX, &CPU::ABS, 4};

    // --- CPY ---
    lookup[0xC0] = {"CPY_imm", &CPU::CPY, &CPU::IMM, 2};
    lookup[0xC4] = {"CPY_zp",  &CPU::CPY, &CPU::ZP0, 3};
    lookup[0xCC] = {"CPY_abs", &CPU::CPY, &CPU::ABS, 4};

    // --- Branches (REL, 2 cycles base) ---
    lookup[0xF0] = {"BEQ", &CPU::BEQ, &CPU::REL, 2};
    lookup[0xD0] = {"BNE", &CPU::BNE, &CPU::REL, 2};
    lookup[0xB0] = {"BCS", &CPU::BCS, &CPU::REL, 2};
    lookup[0x90] = {"BCC", &CPU::BCC, &CPU::REL, 2};
    lookup[0x30] = {"BMI", &CPU::BMI, &CPU::REL, 2};
    lookup[0x10] = {"BPL", &CPU::BPL, &CPU::REL, 2};
    lookup[0x70] = {"BVS", &CPU::BVS, &CPU::REL, 2};
    lookup[0x50] = {"BVC", &CPU::BVC, &CPU::REL, 2};

    // --- Stack operations ---
    lookup[0x48] = {"PHA", &CPU::PHA, &CPU::IMP, 3};
    lookup[0x68] = {"PLA", &CPU::PLA, &CPU::IMP, 4};
    lookup[0x08] = {"PHP", &CPU::PHP, &CPU::IMP, 3};
    lookup[0x28] = {"PLP", &CPU::PLP, &CPU::IMP, 4};

    // --- Flag operations ---
    lookup[0x38] = {"SEC", &CPU::SEC, &CPU::IMP, 2};
    lookup[0x18] = {"CLC", &CPU::CLC, &CPU::IMP, 2};
    lookup[0x78] = {"SEI", &CPU::SEI, &CPU::IMP, 2};
    lookup[0x58] = {"CLI", &CPU::CLI, &CPU::IMP, 2};
    lookup[0xF8] = {"SED", &CPU::SED, &CPU::IMP, 2};
    lookup[0xD8] = {"CLD", &CPU::CLD, &CPU::IMP, 2};
    lookup[0xB8] = {"CLV", &CPU::CLV, &CPU::IMP, 2};
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

Word CPU::Fetch_Word() {
    Byte lo = mem->ReadByte(AbsAddr);
    Byte hi = mem->ReadByte(AbsAddr + 1);
    return (hi << 8) | lo;
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
uint8_t CPU::JMP() {
    PC = AbsAddr;
    return 0;
}
uint8_t CPU::JSR() {
    Word str_pc = PC - 1;
    mem->WriteByte(0x0100 + SP--, (str_pc >> 8));
    mem->WriteByte(0x0100 + SP--, (str_pc & 0xFF));
    PC = AbsAddr;
    return 0;
}
uint8_t CPU::RTS() {
    Byte lo = mem->ReadByte(0x0100 + ++SP);
    Byte hi = mem->ReadByte(0x0100 + ++SP);
    PC = ((hi << 8) | lo) + 1;
    return 0;
}
uint8_t CPU::AND() {


    A = A & (mem->ReadByte(AbsAddr));
    setflag((N) , A & 0x80 );
    setflag((Z) , A ==0);



    return 1;
}
uint8_t CPU::ORA() {

    A = A | (mem->ReadByte(AbsAddr));
    setflag((N) , A & 0x80 );
    setflag((Z) , A ==0);



    return 1;

}
uint8_t CPU::EOR() {

    A = A ^ (mem->ReadByte(AbsAddr));
    setflag((N) , A & 0x80 );
    setflag((Z) , A ==0);




    return 1;
}



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


uint8_t CPU::TAX() {
    X = A;

    setflag(Z, X == 0x00);
    setflag(N, X & 0x80);

    return 0;
}
uint8_t CPU::TXA() {
    A = X;

    setflag(Z, A == 0x00);
    setflag(N, A & 0x80);
    return 0;
}
uint8_t CPU::TAY() {
    Y = A;

    setflag(Z, Y == 0x00);
    setflag(N, Y & 0x80);




    return 0;
}
uint8_t CPU::TYA() {

    A = Y;
    setflag(Z, A == 0x00);
    setflag(N, A & 0x80);

    return 0;
}
uint8_t CPU::TXS() {
    SP = X;
    return 0;
}
uint8_t CPU::TSX() {

    X = SP;
    setflag(Z, X == 0x00);
    setflag(N, X & 0x80);

    return 0;
}


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