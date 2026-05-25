#include "Memory.h"



Memory::Memory() {
    mem.fill(0);
}
Memory::~Memory() = default;

bool Memory::LoadImage(const std::filesystem::path& path, Word startAddr) {
    if (!std::filesystem::exists(path))
        throw std::runtime_error("File not found: " + path.string());

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        throw std::runtime_error("Failed to open: " + path.string());

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (startAddr + size > mem.size())
        throw std::runtime_error("Image too large for address space");

    file.read(reinterpret_cast<char*>(mem.data() + startAddr), size);
    return file.good();
}

Byte Memory::ReadByte(Word addr) const {
    assert(addr < MAX_MEM);
    return mem[addr];
}

void Memory::WriteByte(Word addr, Byte data) {
    assert(addr < MAX_MEM);
    mem[addr] = data;
}