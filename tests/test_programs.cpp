//
// Created by Amir Diaa on 12/06/2026.
//


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
