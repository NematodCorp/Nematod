/*
cnrom.cpp

Copyright (c) 30 Yann BOUCHER (yann)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "mappers/include/uxrom.hpp"

#include "nes.hpp"

#include <cstring>
#include <cassert>

#include "nes.hpp"
#include "ppu/include/ppu.hpp"

namespace
{

class Register : public MemoryInterfaceable {
public:
    Register() : MemoryInterfaceable(0x8000) {};
    virtual data  read(address) override { return 0; };
    virtual void write(address addr, data value) override
    {
        uxrom.register_write(0x8000 | addr, value);
    };
public:
};

}

UxROM uxrom;

static ROMBankWindow<0x4000> prg_rom_bank_lo;
static ROMBankWindow<0x4000> prg_rom_bank_hi;
static RAM<0x2000> chr_ram;
static Register register_memory;

void UxROM::init(const cartridge_data& cart)
{
    prg_rom = cart.prg_rom;

    NES::cpu_space.add_read_port(memory_port{&prg_rom_bank_lo , 0x8000});
    NES::cpu_space.add_read_port(memory_port{&prg_rom_bank_hi , 0xC000});
    NES::cpu_space.add_write_port(memory_port{&register_memory, 0x8000});

    NES::ppu.addr_space.add_port(memory_port{&chr_ram, 0x0000});

    prg_rom_bank_lo.set_rom_base(prg_rom.data(), prg_rom.size());
    prg_rom_bank_hi.set_rom_base(prg_rom.data(), prg_rom.size());

    prg_rom_bank_hi.set_bank(prg_rom_bank_hi.bank_count() - 1);

    if (cart.mirroring == cartridge_data::Horizontal)
    {
        NES::set_mirroring(NES::horizontal);
    }
    else if (cart.mirroring == cartridge_data::Vertical)
    {
        // V-mirroring
        NES::set_mirroring(NES::vertical);
    }

    handle_bus_conflicts = cart.submapper == 2;
}

void UxROM::register_write(uint16_t addr, uint8_t val)
{
    if (handle_bus_conflicts)
        val &= NES::cpu_space.read(addr);

    prg_rom_bank_lo.set_bank(val&0b1111);
}
