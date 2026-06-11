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
// LDA Zero Page  (opcode 0xA6, 3 cycles)
// ---------------------------------------------------------------

TEST(LDA, ZeroPageLoadsCorrectValue) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x37);  // value sitting in zero page
    tb.load({ 0xA6, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x37);
}

TEST(LDA, ZeroPageSetsZeroFlag) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x00);
    tb.load({ 0xA6, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ZeroPageSetsNegativeFlag) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x80);  // bit 7 set
    tb.load({ 0xA6, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x80);
    EXPECT_TRUE(tb.cpu.getflag(CPU::N));
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
}

TEST(LDA, ZeroPageClearsFlagsForPositiveValue) {
    TestBench tb;
    tb.mem.WriteByte(0x0042, 0x42);  // neither zero nor negative
    tb.load({ 0xA6, 0x42 });         // LDA $42
    tb.run(3);
    EXPECT_FALSE(tb.cpu.getflag(CPU::Z));
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(LDA, ZeroPageReadsFromZeroPageNotAbsolute) {
    TestBench tb;
    tb.mem.WriteByte(0x0080, 0x11);  // the real target in zero page
    tb.mem.WriteByte(0x8080, 0x99);  // decoy at the absolute address
    tb.load({ 0xA6, 0x80 });         // LDA $80
    tb.run(3);
    EXPECT_EQ(tb.cpu.A, 0x11);
}