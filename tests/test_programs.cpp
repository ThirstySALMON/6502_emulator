// Multi-instruction program tests that verify correctness AND total cycle counts
// via cpu.get_elapsed_cycles(). After reset the counter starts at 7.

#include <gtest/gtest.h>
#include "CPU.h"
#include "Memory.h"

struct TestBench {
    Memory mem;
    CPU    cpu;

    TestBench() {
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
// Baseline: reset itself consumes 7 cycles
// ---------------------------------------------------------------

TEST(Programs, ResetConsumesSevenCycles) {
    TestBench tb;
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 7);
}

// ---------------------------------------------------------------
// NOP chain — the simplest multi-instruction cycle sanity check
// 3 × NOP (2) = 6 instruction cycles → total 13
// ---------------------------------------------------------------

TEST(Programs, ThreeNOPsCycles) {
    TestBench tb;
    tb.load({ 0xEA, 0xEA, 0xEA });
    tb.run(6);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 13); // 7 + 6
}

// ---------------------------------------------------------------
// Load all three registers
// LDA IMM (2) + LDX IMM (2) + LDY IMM (2) = 6 cycles → total 13
// ---------------------------------------------------------------

TEST(Programs, LoadAllRegisters) {
    TestBench tb;
    tb.load({
        0xA9, 0x10,   // LDA #$10 (2)
        0xA2, 0x20,   // LDX #$20 (2)
        0xA0, 0x30,   // LDY #$30 (2)
    });
    tb.run(6);
    EXPECT_EQ(tb.cpu.A, 0x10);
    EXPECT_EQ(tb.cpu.X, 0x20);
    EXPECT_EQ(tb.cpu.Y, 0x30);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 13); // 7 + 6
}

// ---------------------------------------------------------------
// Store and load via memory
// LDA IMM (2) + STA ZP (3) + LDX ZP (3) = 8 cycles → total 15
// ---------------------------------------------------------------

TEST(Programs, StoreAndLoadViaMemory) {
    TestBench tb;
    tb.load({
        0xA9, 0x55,   // LDA #$55 (2)
        0x85, 0x10,   // STA $10  (3) — zero-page store
        0xA6, 0x10,   // LDX $10  (3) — zero-page load
    });
    tb.run(8);
    EXPECT_EQ(tb.cpu.A, 0x55);
    EXPECT_EQ(tb.cpu.X, 0x55);
    EXPECT_EQ(tb.mem.ReadByte(0x0010), 0x55);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 15); // 7 + 8
}

// ---------------------------------------------------------------
// Counter program: LDX + 3×INX + CPX
// LDX IMM (2) + 3×INX (6) + CPX IMM (2) = 10 cycles → total 17
// ---------------------------------------------------------------

TEST(Programs, IncrementCounterCorrectResult) {
    TestBench tb;
    tb.load({
        0xA2, 0x00,   // LDX #$00 (2)
        0xE8,         // INX      (2)
        0xE8,         // INX      (2)
        0xE8,         // INX      (2)
        0xE0, 0x03,   // CPX #$03 (2)
    });
    tb.run(10);
    EXPECT_EQ(tb.cpu.X, 0x03);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));   // X == 3
    EXPECT_TRUE(tb.cpu.getflag(CPU::C));   // X >= 3
    EXPECT_FALSE(tb.cpu.getflag(CPU::N));
}

TEST(Programs, IncrementCounterTotalCycles) {
    TestBench tb;
    tb.load({
        0xA2, 0x00,
        0xE8, 0xE8, 0xE8,
        0xE0, 0x03,
    });
    tb.run(10);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 17); // 7 + 10
}

// ---------------------------------------------------------------
// Decrement countdown: LDY #$03, 3×DEY, CPY #$00
// LDY IMM (2) + 3×DEY (6) + CPY IMM (2) = 10 cycles → total 17
// ---------------------------------------------------------------

TEST(Programs, DecrementCountdownCorrectResult) {
    TestBench tb;
    tb.load({
        0xA0, 0x03,   // LDY #$03 (2)
        0x88,         // DEY      (2)
        0x88,         // DEY      (2)
        0x88,         // DEY      (2)
        0xC0, 0x00,   // CPY #$00 (2)
    });
    tb.run(10);
    EXPECT_EQ(tb.cpu.Y, 0x00);
    EXPECT_TRUE(tb.cpu.getflag(CPU::Z));
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 17); // 7 + 10
}

// ---------------------------------------------------------------
// Stack round-trip: LDA + PHA + LDA + PLA
// LDA IMM (2) + PHA (3) + LDA IMM (2) + PLA (4) = 11 cycles → total 18
// ---------------------------------------------------------------

TEST(Programs, StackRoundTripCorrectResult) {
    TestBench tb;
    tb.load({
        0xA9, 0x42,   // LDA #$42 (2)
        0x48,         // PHA      (3)
        0xA9, 0x00,   // LDA #$00 (2) — clobber A
        0x68,         // PLA      (4) — restore A
    });
    tb.run(11);
    EXPECT_EQ(tb.cpu.A, 0x42);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 18); // 7 + 11
}

// ---------------------------------------------------------------
// JSR + RTS subroutine call
//   $8000: JSR $8010 (6) → jump into subroutine
//   $8010: NOP       (2) → body of subroutine
//   $8011: RTS       (6) → return to $8003
//   $8003: NOP       (2) → first instruction after return
//   Total instruction cycles: 6 + 2 + 6 + 2 = 16 → total 23
// ---------------------------------------------------------------

TEST(Programs, JSRRTSLandsAtCorrectAddress) {
    TestBench tb;
    tb.mem.LoadBytes({ 0x20, 0x10, 0x80,  // JSR $8010 (6)
                       0xEA },             // NOP       (2) — after return
                     0x8000);
    tb.mem.LoadBytes({ 0xEA,              // NOP       (2) — subroutine body
                       0x60 },            // RTS       (6)
                     0x8010);
    tb.run(16);
    EXPECT_EQ(tb.cpu.PC, 0x8004);         // NOP at $8003 advanced PC to $8004
}

TEST(Programs, JSRRTSTotalCycles) {
    TestBench tb;
    tb.mem.LoadBytes({ 0x20, 0x10, 0x80,  // JSR $8010 (6)
                       0xEA },             // NOP       (2)
                     0x8000);
    tb.mem.LoadBytes({ 0xEA,              // NOP       (2)
                       0x60 },            // RTS       (6)
                     0x8010);
    tb.run(16);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 23); // 7 + 16
}

// ---------------------------------------------------------------
// Page-cross penalty on LDA ABX
// No cross: LDA $1200,X (X=1) → $1201 — 4 cycles
// Cross:    LDA $12FF,X (X=1) → $1300 — 5 cycles (+1)
// ---------------------------------------------------------------

TEST(Programs, NoPageCrossNoPenalty) {
    TestBench tb;
    tb.cpu.X = 0x01;
    tb.mem.WriteByte(0x1201, 0xCD);
    tb.load({ 0xBD, 0x00, 0x12 });     // LDA $1200,X (same page → 4 cycles)
    tb.run(4);
    EXPECT_EQ(tb.cpu.A, 0xCD);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 11); // 7 + 4
}

TEST(Programs, PageCrossPenaltyAddsOneCycle) {
    TestBench tb;
    tb.cpu.X = 0x01;
    tb.mem.WriteByte(0x1300, 0xAB);
    tb.load({ 0xBD, 0xFF, 0x12 });     // LDA $12FF,X (crosses to $1300 → 5 cycles)
    tb.run(5);
    EXPECT_EQ(tb.cpu.A, 0xAB);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 12); // 7 + 5
}

// ---------------------------------------------------------------
// Flag ops: SEC + CLC + SEI + CLI = 8 cycles → total 15
// ---------------------------------------------------------------

TEST(Programs, FlagOpsCyclesAndResult) {
    TestBench tb;
    tb.load({
        0x38,   // SEC (2) — C=1
        0x18,   // CLC (2) — C=0
        0x78,   // SEI (2) — I=1
        0x58,   // CLI (2) — I=0
    });
    tb.run(8);
    EXPECT_FALSE(tb.cpu.getflag(CPU::C));
    EXPECT_FALSE(tb.cpu.getflag(CPU::I));
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 15); // 7 + 8
}

// ---------------------------------------------------------------
// INC + DEC memory round-trip via zero page
// INC ZP (5) + DEC ZP (5) = 10 cycles → total 17
// ---------------------------------------------------------------

TEST(Programs, IncThenDecLeavesValueUnchanged) {
    TestBench tb;
    tb.mem.WriteByte(0x0020, 0x42);
    tb.load({
        0xE6, 0x20,   // INC $20 (5)
        0xC6, 0x20,   // DEC $20 (5)
    });
    tb.run(10);
    EXPECT_EQ(tb.mem.ReadByte(0x0020), 0x42);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 17); // 7 + 10
}

// ---------------------------------------------------------------
// Transfer chain: A → X → SP → X → A
// LDA IMM (2) + TAX (2) + TXS (2) + TSX (2) + TXA (2) = 10 cycles → total 17
// ---------------------------------------------------------------

TEST(Programs, TransferChainCyclesAndResult) {
    TestBench tb;
    tb.load({
        0xA9, 0x42,   // LDA #$42 (2)
        0xAA,         // TAX      (2) — X=$42
        0x9A,         // TXS      (2) — SP=$42
        0xBA,         // TSX      (2) — X=SP=$42
        0x8A,         // TXA      (2) — A=$42
    });
    tb.run(10);
    EXPECT_EQ(tb.cpu.A,  0x42);
    EXPECT_EQ(tb.cpu.X,  0x42);
    EXPECT_EQ(tb.cpu.SP, 0x42);
    EXPECT_EQ(tb.cpu.get_elapsed_cycles(), 17); // 7 + 10
}
