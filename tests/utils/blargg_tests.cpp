/*
blargg_tests.cpp

Copyright (c) 22 Yann BOUCHER (yann)

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

#include "blargg_tests.hpp"

#include "nes.hpp"
#include "ppu.hpp"
#include "inputadapter.hpp"
#include "standard_controller.hpp"
#include "nesloader.hpp"

RAM<0x400 > ppu_nt1;
RAM<0x400 > ppu_nt2;
ROM<0x8000> prg_rom;
ROM<0x2000> chr_rom;
RAM<0x2000> chr_ram;
RAM<0x2000> crt_ram;

StandardController controller_1;

bool do_blargg_test(const std::string& rom_path, std::string& output)
{
    cartdrige_data cart;
    cart = load_nes_file(rom_path);

    NES::init();
    NES::input.controller_1 = &controller_1;

    memcpy(prg_rom.m_data.data(), cart.prg_rom.data(), cart.prg_rom.size());
    if (cart.prg_rom.size() == 0x4000)
    {
        memcpy(prg_rom.m_data.data() + 0x4000, cart.prg_rom.data(), cart.prg_rom.size());
    }

    if (!cart.chr_rom.empty())
        memcpy(chr_rom.m_data.data(), cart.chr_rom.data(), 0x2000);

    NES::cpu_space.add_port(memory_port{&crt_ram,  0x6000});
    NES::cpu_space.add_port(memory_port{&prg_rom , 0x8000});

    if (!cart.chr_rom.empty())
        NES::ppu.addr_space.add_port(memory_port{&chr_rom, 0x0000});
    else
        NES::ppu.addr_space.add_port(memory_port{&chr_ram, 0x0000});

    if (cart.mirroring == cartdrige_data::Horizontal)
    {
        NES::set_mirroring(NES::horizontal);
    }
    else if (cart.mirroring == cartdrige_data::Vertical)
    {
        // V-mirroring
        NES::set_mirroring(NES::vertical);
    }

    NES::power_cycle();

    crt_ram.m_internal[0x0] = 0x80;



    while(crt_ram.m_internal[0x0] == 0x80)
    {
        NES::run_cpu_cycle();
        //NES::run_frame();
    }

    output = std::string((char*)&crt_ram.m_internal[0x4]);

    if (crt_ram.m_internal[0x0] == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
