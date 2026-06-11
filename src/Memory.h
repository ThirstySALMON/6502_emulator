/*Memory class
 *
 *Provide abstraction for Memory access
 *Technically also a bus
 *Public functions:
 *Read_byte(Word w) -- fetches a byte at address w
 *Write_byte(Word w , Byte data ) --  stores data at memory location w
 *Disk_load()
 *
 *Private Functions:
 *
 *Zero init
 *
 *
 */

#ifndef INC_6502_EMULATOR_MEMORY_H
#define INC_6502_EMULATOR_MEMORY_H


# include "incl.h"



class Memory {

private:
    // Constants
    static constexpr int MAX_MEM = 65536;


    std::array<uint8_t, MAX_MEM> mem{};  // {} zero-initializes everything

public:
    Memory();
    bool LoadImage(const std::filesystem::path& path, Word startAddr); // load memory image from file
    Byte ReadByte(Word addr) const;
    void WriteByte(Word addr, Byte data);
    void LoadBytes(std::initializer_list<Byte> , Word );


    ~Memory();




};


#endif //INC_6502_EMULATOR_MEMORY_H