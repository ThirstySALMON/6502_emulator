#include <iostream>
#include "Memory.h"
#include "CPU.h"

void printCpuState(const CPU& cpu) {
    std::cout << "A: 0x" << std::hex << (int)cpu.A
              << " X: 0x" << (int)cpu.X
              << " Y: 0x" << (int)cpu.Y
              << " SP: 0x" << (int)cpu.SP
              << " PC: 0x" << cpu.PC
              << std::dec << std::endl;
    std::cout << "Flags: "
              << (cpu.getflag(CPU::N) ? "N" : "-")
              << (cpu.getflag(CPU::V) ? "V" : "-")
              << (cpu.getflag(CPU::U) ? "U" : "-")
              << (cpu.getflag(CPU::B) ? "B" : "-")
              << (cpu.getflag(CPU::D) ? "D" : "-")
              << (cpu.getflag(CPU::I) ? "I" : "-")
              << (cpu.getflag(CPU::Z) ? "Z" : "-")
              << (cpu.getflag(CPU::C) ? "C" : "-")
              << std::endl;
}

int main() {
    Memory memory;
    CPU cpu;

    // Set reset vector to point to 0x8000 ( program start)
    memory.WriteByte(0xFFFC, 0x00);
    memory.WriteByte(0xFFFD, 0x80);

    // Write a simple test program at 0x8000
    Word addr = 0x8000;
    memory.WriteByte(addr++, 0xA9);  // LDA #$42
    memory.WriteByte(addr++, 0x42);
    memory.WriteByte(addr++, 0xEA);  // NOP
    memory.WriteByte(addr++, 0xA9);  // LDA #$00 (test zero flag)
    memory.WriteByte(addr++, 0x00);
    memory.WriteByte(addr++, 0xA9);  // LDA #$80 (test negative flag)
    memory.WriteByte(addr++, 0x80);

    std::cout << "=== 6502 Emulator Test ===" << std::endl << std::endl;

    // Reset CPU (reads reset vector, sets PC to 0x8000)
    cpu.reset(memory);
    std::cout << "After reset:" << std::endl;
    printCpuState(cpu);
    std::cout << std::endl;

    // Execute: LDA #$42 (2 cycles)
    std::cout << "Executing LDA #$42..." << std::endl;
    cpu.clock(memory, 2);
    printCpuState(cpu);
    std::cout << std::endl;

    // Execute: NOP (2 cycles)
    std::cout << "Executing NOP..." << std::endl;
    cpu.clock(memory, 2);
    printCpuState(cpu);
    std::cout << std::endl;

    // Execute: LDA #$00 (2 cycles) - should set Zero flag
    std::cout << "Executing LDA #$00 (should set Z flag)..." << std::endl;
    cpu.clock(memory, 2);
    printCpuState(cpu);
    std::cout << std::endl;

    // Execute: LDA #$80 (2 cycles) - should set Negative flag
    std::cout << "Executing LDA #$80 (should set N flag)..." << std::endl;
    cpu.clock(memory, 2);
    printCpuState(cpu);

    std::cout << std::endl << "=== Test Complete ===" << std::endl;

    return 0;
}
