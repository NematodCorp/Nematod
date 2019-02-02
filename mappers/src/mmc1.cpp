/*
mmc1.cpp

Copyright (c) 23 Yann BOUCHER (yann)

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

#include "mappers/include/mmc1.hpp"

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
        mmc1.register_write(0x8000 | addr, value);
    };
public:
};

}

MMC1 mmc1;

static RAM<0x2000> crt_ram;
static ROMBankWindow<0x4000> prg_bank_low;
static ROMBankWindow<0x4000> prg_bank_hi;
static RAMBankWindow<0x1000> chr_bank_low;
static RAMBankWindow<0x1000> chr_bank_hi;
static Register register_memory;

void MMC1::init(const cartridge_data& cart)
{
    bool uses_chr_ram { false };

    handle_bus_conflicts = cart.submapper == 5; // SEROM
    prg_rom = cart.prg_rom;
    chr_rom = cart.chr_rom;
    if (chr_rom.size() == 0) // use CHR RAM
    {
        uses_chr_ram = true;
        chr_rom.resize(0x2000);
    }

    prg_bank_low.set_rom_base(prg_rom.data(), prg_rom.size());
    prg_bank_hi.set_rom_base(prg_rom.data(), prg_rom.size());
    chr_bank_low.set_rom_base(chr_rom.data(), chr_rom.size());
    chr_bank_hi.set_rom_base(chr_rom.data(), chr_rom.size());

    NES::cpu_space.add_port(memory_port{&crt_ram,  0x6000});
    NES::cpu_space.add_read_port(memory_port{&prg_bank_low, 0x8000});
    NES::cpu_space.add_read_port(memory_port{&prg_bank_hi , 0xC000});
    NES::cpu_space.add_write_port(memory_port{&register_memory, 0x8000});

    NES::ppu.addr_space.add_read_port(memory_port{&chr_bank_low, 0x0000});
    NES::ppu.addr_space.add_read_port(memory_port{&chr_bank_hi , 0x1000});
    if (uses_chr_ram) // enable writes
    {
        NES::ppu.addr_space.add_write_port(memory_port{&chr_bank_low, 0x0000});
        NES::ppu.addr_space.add_write_port(memory_port{&chr_bank_hi , 0x1000});
    }

    if (cart.mirroring == cartridge_data::Horizontal)
    {
        NES::set_mirroring(NES::horizontal);
    }
    else if (cart.mirroring == cartridge_data::Vertical)
    {
        // V-mirroring
        NES::set_mirroring(NES::vertical);
    }

    ctrl_reg = 0x0C;
    chr0_reg = chr1_reg = prg_reg = 0;

    apply_banking();
}

void MMC1::register_write(uint16_t addr, uint8_t val)
{
    // bus conflicts
    if (handle_bus_conflicts)
    {
        val &= NES::cpu_space.read(addr);
    }

    // ignore successive writes to prevent RMW instructions to alter the register twice due to dummy accesses
    if (NES::cpu.cycles <= last_write_cycle+2)
    {
        return;
    }
    last_write_cycle = NES::cpu.cycles;

    if (val & 0x80) // bit 7 set
    {
        shift_register = write_count = 0;
        ctrl_reg |= 0x0C; // lock PRG ROM at $C000-$FFFF
        apply_banking();
    }
    else
    {
        shift_register >>= 1;
        shift_register |= (val&1) << 4;
        write_count++;

        if (write_count == 5)
        {
            // bits 13 and 14
            switch ((addr>>13)&0b11)
            {
                case 0:
                    ctrl_reg = shift_register;
                    break;
                case 1:
                    chr0_reg = shift_register;
                    break;
                case 2:
                    chr1_reg = shift_register;
                    break;
                case 3:
                    prg_reg  = shift_register;
                    break;
            }

            apply_banking();

            shift_register = write_count = 0;
        }
    }
}

void MMC1::load_battery_ram(const std::vector<uint8_t> &data)
{
    assert(data.size() == crt_ram.size());
    memcpy(crt_ram.m_internal.data(), data.data(), data.size());
}

std::vector<uint8_t> MMC1::save_battery_ram()
{
    std::vector<uint8_t> out;
    out.resize(crt_ram.size());

    memcpy(out.data(), crt_ram.m_internal.data(), crt_ram.size());

    return out;
}

void MMC1::apply_banking()
{
    switch (ctrl_reg&0b11)
    {
        case 0:
            NES::set_mirroring(NES::onescreen_nt1);
            break;
        case 1:
            NES::set_mirroring(NES::onescreen_nt2);
            break;
        case 2:
            NES::set_mirroring(NES::vertical);
            break;
        case 3:
            NES::set_mirroring(NES::horizontal);
            break;
    }

    const uint8_t prg_mode = (ctrl_reg>>2)&0b11;
    const uint8_t chr_mode = (ctrl_reg>>4)&0b1;

    if (chr_mode == 1) // 4 KB mode
    {
        chr_bank_low.set_bank(chr0_reg);
        chr_bank_hi.set_bank(chr1_reg);
    }
    else               // 8 KB mode
    {
        uint8_t bank = (chr0_reg&0b11110); // ignore low bit
        chr_bank_low.set_bank(bank);
        chr_bank_hi.set_bank(bank+1);
    }

    uint8_t bank = (prg_reg&0b1111);

    switch (prg_mode)
    {
        case 0:
        case 1:
            bank = (bank&0b1110); // low bit ignored in 32 KB mode
            prg_bank_low.set_bank(bank);
            prg_bank_hi.set_bank(bank+1);
            break;
        case 2:
            prg_bank_low.set_bank(0); // first bank tp $8000
            prg_bank_hi.set_bank(bank);
            break;
        case 3:
            prg_bank_low.set_bank(bank);
            prg_bank_hi.set_bank(prg_bank_hi.bank_count() - 1);  // last  bank to $C000
            break;
    }
    // enable PRG RAM
    bool prg_ram_enabled = (prg_reg&0b10000) == 0;
    crt_ram.set_valid(prg_ram_enabled);
}
