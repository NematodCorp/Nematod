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

#include "mappers/include/cnrom.hpp"

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
        cnrom.register_write(0x8000 | addr, value);
    };
public:
};

}

CNROM cnrom;

static ROM<0x8000> prg_rom;
static ROMBankWindow<0x2000> chr_bank;
static Register register_memory;

void CNROM::init(const cartridge_data& cart)
{
    memcpy(prg_rom.m_data.data(), cart.prg_rom.data(), cart.prg_rom.size());
    if (cart.prg_rom.size() == 0x4000) // mirroring
    {
        memcpy(prg_rom.m_data.data() + 0x4000, cart.prg_rom.data(), cart.prg_rom.size());
    }

    chr_rom = cart.chr_rom;

    chr_bank.set_rom_base(chr_rom.data(), chr_rom.size());
    chr_bank.set_bank(0);

    NES::cpu_space.add_read_port(memory_port{&prg_rom, 0x8000});
    NES::cpu_space.add_write_port(memory_port{&register_memory, 0x8000});

    NES::ppu.addr_space.add_read_port(memory_port{&chr_bank, 0x0000});

    if (cart.mirroring == cartridge_data::Horizontal)
    {
        NES::set_mirroring(NES::horizontal);
    }
    else if (cart.mirroring == cartridge_data::Vertical)
    {
        // V-mirroring
        NES::set_mirroring(NES::vertical);
    }

    if (cart.submapper != 1)
        handle_bus_conflicts = true;
    else
        handle_bus_conflicts = false;
}

void CNROM::register_write(uint16_t addr, uint8_t val)
{
    if (handle_bus_conflicts)
        val &= NES::cpu_space.read(addr);

    chr_bank.set_bank(val&0b11);
}
