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

#include <cassert>

#include "nes.hpp"
#include "ppu.hpp"
#include "inputadapter.hpp"
#include "standard_controller.hpp"
#include "nesloader.hpp"

StandardController controller_1;

bool do_blargg_test(const std::string& rom_path, std::string& output)
{
    NES::init();
    assert(NES::load_cartridge(rom_path));
    NES::input.controller_1 = &controller_1;

    NES::power_cycle();

    NES::cpu.write(0x6000, 0x80);

    while(NES::cpu.read(0x6000) == 0x80)
    {
        NES::run_cpu_cycle();
    }

    if (NES::cpu.read(0x6000) == 0)
    {
        return true;
    }
    else
    {
        int txt_idx = 0x6004;
        char c;
        while ((c = (char)NES::cpu.read(txt_idx++)))
            output += c;
        return false;
    }
}
