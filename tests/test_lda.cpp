//
// Created by Amir Diaa on 11/06/2026.
//

#include <gtest/gtest.h>
#include "CPU.h"
#include "Memory.h"

// Helper — sets up memory and runs the CPU for n cycles
struct TestBench {
    Memory mem;
    CPU    cpu;

    TestBench() {
        // point reset vector to $8000 where we load test programs
        mem.WriteByte(0xFFFC, 0x00);
        mem.WriteByte(0xFFFD, 0x80);
        cpu.reset(mem);
    }

    void load(std::initializer_list<Byte> bytes) {
        mem.LoadBytes(bytes, 0x8000);
    }

    void run(int32_t cycles) {
        cpu.clock(mem, cycles);
    }
};

// ---------------------------------------------------------------
// LDA Immediate
// ---------------------------------------------------------------

TEST(LDA, ImmediateLoadsCorrectValue) {
    TestBench tb;
    tb.load({ 0xA9, 0x42 });  // LDA #$42
    tb.run(2);
    EXPECT_EQ(tb.cpu.A, 0x42);
}

TEST(LDA, ImmediateSetsZeroFlag) {
    TestBench tb;
    tb.load({ 0xA9, 0x00 });  // LDA #$00
    tb.run(2);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ImmediateSetsNegativeFlag) {
    TestBench tb;
    tb.load({ 0xA9, 0xFF });  // LDA #$FF — bit 7 set
    tb.run(2);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, ImmediateClearsFlagsForPositiveValue) {
    TestBench tb;
    tb.load({ 0xA9, 0x42 });  // LDA #$42 — neither zero nor negative
    tb.run(2);
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

// ---------------------------------------------------------------
// LDA Zero Page  (opcode 0xA5, 3 cycles)
// ---------------------------------------------------------------

TEST(LDA, ZeroPageLoadsCorrectValue) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x37);  // value sitting in zero page
    tb.load({ 0xA5, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x37);
}

TEST(LDA, ZeroPageSetsZeroFlag) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x00);
    tb.load({ 0xA5, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ZeroPageSetsNegativeFlag) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x80);  // bit 7 set
    tb.load({ 0xA5, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x80);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, ZeroPageClearsFlagsForPositiveValue) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x42);  // neither zero nor negative
    tb.load({ 0xA5, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ZeroPageReadsFromZeroPageNotAbsolute) {
    TestBench tb;
    tb.mem.WriteByte(0x0080, 0x11);  // the real target in zero page
    tb.mem.WriteByte(0x8080, 0x99);  // decoy at the absolute address
    tb.load({ 0xA5, 0x80 });         // LDA $80
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x11);
}

// ---------------------------------------------------------------
// LDA Absolute  (opcode 0xAD, 4 cycles)
// ---------------------------------------------------------------

TEST(LDA, AbsoluteLoadsCorrectValue) {
    TestBench tb;
    tb.mem.WriteByte(0x1234, 0x55);       // value at target address
    tb.load({ 0xAD, 0x34, 0x12 });        // LDA $1234  (little-endian)
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x55);
}

TEST(LDA, AbsoluteSetsZeroFlag) {
    TestBench tb;
    tb.mem.WriteByte(0x1234, 0x00);
    tb.load({ 0xAD, 0x34, 0x12 });        // LDA $1234
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, AbsoluteSetsNegativeFlag) {
    TestBench tb;
    tb.mem.WriteByte(0x1234, 0xFE);       // bit 7 set
    tb.load({ 0xAD, 0x34, 0x12 });        // LDA $1234
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0xFE);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, AbsoluteClearsFlagsForPositiveValue) {
    TestBench tb;
    tb.mem.WriteByte(0x1234, 0x42);
    tb.load({ 0xAD, 0x34, 0x12 });        // LDA $1234
    tb.run(4);
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, AbsoluteReadsFullAddressLittleEndian) {
    TestBench tb;
    tb.mem.WriteByte(0x4321, 0x77);       // correct target
    tb.mem.WriteByte(0x0043, 0x99);       // decoy: what ZP would load
    tb.load({ 0xAD, 0x21, 0x43 });        // LDA $4321
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x77);
}

// ---------------------------------------------------------------
// LDA Zero Page, X  (opcode 0xB5, 4 cycles)
// Effective address = (operand + X) & 0xFF — always stays in page 0
// ---------------------------------------------------------------

TEST(LDA, ZeroPageXLoadsCorrectValue) {
    TestBench tb;
    tb.cpu.X = 0x05;
    tb.mem.WriteByte(0x0047, 0x33);       // $42 + $05 = $47
    tb.load({ 0xB5, 0x42 });              // LDA $42,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x33);
}

TEST(LDA, ZeroPageXSetsZeroFlag) {
    TestBench tb;
    tb.cpu.X = 0x01;
    tb.mem.WriteByte(0x0043, 0x00);       // $42 + $01 = $43
    tb.load({ 0xB5, 0x42 });              // LDA $42,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ZeroPageXSetsNegativeFlag) {
    TestBench tb;
    tb.cpu.X = 0x02;
    tb.mem.WriteByte(0x0044, 0x80);       // $42 + $02 = $44, bit 7 set
    tb.load({ 0xB5, 0x42 });              // LDA $42,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x80);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, ZeroPageXClearsFlagsForPositiveValue) {
    TestBench tb;
    tb.cpu.X = 0x10;
    tb.mem.WriteByte(0x0052, 0x42);       // $42 + $10 = $52
    tb.load({ 0xB5, 0x42 });              // LDA $42,X
    tb.run(4);
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ZeroPageXWrapsAroundZeroPage) {
    TestBench tb;
    tb.cpu.X = 0x20;
    // ($F0 + $20) & 0xFF = $10 — must NOT read from $0110
    tb.mem.WriteByte(0x0010, 0xAB);       // correct: wrapped target
    tb.mem.WriteByte(0x0110, 0xCC);       // decoy: unwrapped absolute address
    tb.load({ 0xB5, 0xF0 });              // LDA $F0,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0xAB);
}

// ---------------------------------------------------------------
// LDA Absolute, X  (opcode 0xBD, 4 cycles; +1 on page cross)
// Effective address = base + X
// ---------------------------------------------------------------

TEST(LDA, AbsoluteXLoadsCorrectValue) {
    TestBench tb;
    tb.cpu.X = 0x05;
    tb.mem.WriteByte(0x1205, 0x44);       // $1200 + $05 = $1205
    tb.load({ 0xBD, 0x00, 0x12 });        // LDA $1200,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x44);
}

TEST(LDA, AbsoluteXSetsZeroFlag) {
    TestBench tb;
    tb.cpu.X = 0x01;
    tb.mem.WriteByte(0x1201, 0x00);
    tb.load({ 0xBD, 0x00, 0x12 });        // LDA $1200,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, AbsoluteXSetsNegativeFlag) {
    TestBench tb;
    tb.cpu.X = 0x02;
    tb.mem.WriteByte(0x1202, 0xFF);       // bit 7 set
    tb.load({ 0xBD, 0x00, 0x12 });        // LDA $1200,X
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0xFF);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, AbsoluteXPageCrossReadsCorrectValue) {
    TestBench tb;
    tb.cpu.X = 0x10;
    // $12F5 + $10 = $1305 — crosses from page $12 to page $13
    tb.mem.WriteByte(0x1305, 0x55);
    tb.load({ 0xBD, 0xF5, 0x12 });        // LDA $12F5,X
    tb.run(5);                             // 4 base + 1 page-cross penalty
    EXPECT_EQ(tb.cpu.A, 0x55);
}

// ---------------------------------------------------------------
// LDA Absolute, Y  (opcode 0xB9, 4 cycles; +1 on page cross)
// Effective address = base + Y
// ---------------------------------------------------------------

TEST(LDA, AbsoluteYLoadsCorrectValue) {
    TestBench tb;
    tb.cpu.Y = 0x03;
    tb.mem.WriteByte(0x1203, 0x66);       // $1200 + $03 = $1203
    tb.load({ 0xB9, 0x00, 0x12 });        // LDA $1200,Y
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x66);
}

TEST(LDA, AbsoluteYSetsZeroFlag) {
    TestBench tb;
    tb.cpu.Y = 0x04;
    tb.mem.WriteByte(0x1204, 0x00);
    tb.load({ 0xB9, 0x00, 0x12 });        // LDA $1200,Y
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, AbsoluteYSetsNegativeFlag) {
    TestBench tb;
    tb.cpu.Y = 0x05;
    tb.mem.WriteByte(0x1205, 0x81);       // bit 7 set
    tb.load({ 0xB9, 0x00, 0x12 });        // LDA $1200,Y
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0x81);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, AbsoluteYPageCrossReadsCorrectValue) {
    TestBench tb;
    tb.cpu.Y = 0x20;
    // $12E0 + $20 = $1300 — crosses from page $12 to page $13
    tb.mem.WriteByte(0x1300, 0x77);
    tb.load({ 0xB9, 0xE0, 0x12 });        // LDA $12E0,Y
    tb.run(5);                             // 4 base + 1 page-cross penalty
    EXPECT_EQ(tb.cpu.A, 0x77);
}

// ---------------------------------------------------------------
// LDA (Indirect, X)  (opcode 0xA1, 6 cycles)
// ptr = (operand + X) & 0xFF; effective address = mem[ptr] | mem[ptr+1]<<8
// ---------------------------------------------------------------

TEST(LDA, IndirectXLoadsCorrectValue) {
    TestBench tb;
    tb.cpu.X = 0x04;
    // ptr = ($20 + $04) & FF = $24; address stored at $0024/$0025 = $1234
    tb.mem.WriteByte(0x0024, 0x34);
    tb.mem.WriteByte(0x0025, 0x12);
    tb.mem.WriteByte(0x1234, 0xAA);
    tb.load({ 0xA1, 0x20 });              // LDA ($20,X)
    tb.run(6);
    EXPECT_EQ(tb.cpu.A, 0xAA);
}

TEST(LDA, IndirectXSetsZeroFlag) {
    TestBench tb;
    tb.cpu.X = 0x02;
    tb.mem.WriteByte(0x0032, 0x00);
    tb.mem.WriteByte(0x0033, 0x20);
    tb.mem.WriteByte(0x2000, 0x00);
    tb.load({ 0xA1, 0x30 });              // LDA ($30,X)  ptr=$32
    tb.run(6);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, IndirectXSetsNegativeFlag) {
    TestBench tb;
    tb.cpu.X = 0x01;
    tb.mem.WriteByte(0x0051, 0x00);
    tb.mem.WriteByte(0x0052, 0x30);
    tb.mem.WriteByte(0x3000, 0x80);
    tb.load({ 0xA1, 0x50 });              // LDA ($50,X)  ptr=$51
    tb.run(6);
    EXPECT_EQ(tb.cpu.A, 0x80);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, IndirectXPtrWrapsInZeroPage) {
    TestBench tb;
    tb.cpu.X = 0x05;
    // ($FE + $05) & FF = $03; pointer at $0003/$0004 = $5678
    tb.mem.WriteByte(0x0003, 0x78);
    tb.mem.WriteByte(0x0004, 0x56);
    tb.mem.WriteByte(0x5678, 0xBB);
    tb.load({ 0xA1, 0xFE });              // LDA ($FE,X)  ptr wraps to $03
    tb.run(6);
    EXPECT_EQ(tb.cpu.A, 0xBB);
}

// ---------------------------------------------------------------
// LDA (Indirect), Y  (opcode 0xB1, 5 cycles; +1 on page cross)
// base = mem[operand] | mem[operand+1]<<8; effective = base + Y
// ---------------------------------------------------------------

TEST(LDA, IndirectYLoadsCorrectValue) {
    TestBench tb;
    tb.cpu.Y = 0x03;
    // pointer at $0050/$0051 = $1200; effective = $1200 + $03 = $1203
    tb.mem.WriteByte(0x0050, 0x00);
    tb.mem.WriteByte(0x0051, 0x12);
    tb.mem.WriteByte(0x1203, 0x99);
    tb.load({ 0xB1, 0x50 });              // LDA ($50),Y
    tb.run(5);
    EXPECT_EQ(tb.cpu.A, 0x99);
}

TEST(LDA, IndirectYSetsZeroFlag) {
    TestBench tb;
    tb.cpu.Y = 0x01;
    tb.mem.WriteByte(0x0060, 0x00);
    tb.mem.WriteByte(0x0061, 0x20);
    tb.mem.WriteByte(0x2001, 0x00);
    tb.load({ 0xB1, 0x60 });              // LDA ($60),Y
    tb.run(5);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, IndirectYSetsNegativeFlag) {
    TestBench tb;
    tb.cpu.Y = 0x02;
    tb.mem.WriteByte(0x0070, 0x00);
    tb.mem.WriteByte(0x0071, 0x40);
    tb.mem.WriteByte(0x4002, 0xFE);
    tb.load({ 0xB1, 0x70 });              // LDA ($70),Y
    tb.run(5);
    EXPECT_EQ(tb.cpu.A, 0xFE);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, IndirectYPageCrossReadsCorrectValue) {
    TestBench tb;
    tb.cpu.Y = 0x10;
    // base = $12F8; $12F8 + $10 = $1308 — page cross
    tb.mem.WriteByte(0x0080, 0xF8);
    tb.mem.WriteByte(0x0081, 0x12);
    tb.mem.WriteByte(0x1308, 0xCC);
    tb.load({ 0xB1, 0x80 });              // LDA ($80),Y
    tb.run(6);                             // 5 base + 1 page-cross penalty
    EXPECT_EQ(tb.cpu.A, 0xCC);
}