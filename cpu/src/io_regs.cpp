/*
io_regs.cpp

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

#include "io_regs.hpp"

#include "ppu/include/ppu.hpp"
#include "cpu.hpp"
#include "input/include/inputadapter.hpp"

#include "common/bitops.hpp"

#include <cassert>

std::array<IORegs::read_callback, 0x20> IORegs::m_read_clbks =
{
    &IORegs::invalid_read,     // PPUCTRL
    &IORegs::invalid_read,     // PPUMASK
    &IORegs::invalid_read , // PPUSTATUS
    &IORegs::invalid_read, // OAMADDR
    &IORegs::invalid_read, // OAMDATA
    &IORegs::invalid_read,   // PPUSCROLL
    &IORegs::invalid_read,     // PPUADDR
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::input_read16,     // INPUTR1
    &IORegs::input_read17,     // INPUTR2
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
    &IORegs::invalid_read,     // PPUDATA
};
std::array<IORegs::write_callback, 0x20> IORegs::m_write_clbks =
{
    &IORegs::invalid_write,     // PPUCTRL
    &IORegs::invalid_write,     // PPUMASK
    &IORegs::invalid_write , // PPUSTATUS
    &IORegs::invalid_write, // OAMADDR
    &IORegs::invalid_write, // OAMDATA
    &IORegs::invalid_write,   // PPUSCROLL
    &IORegs::invalid_write,     // PPUADDR
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::oam_dma,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::input_write,       // INPUTWR
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
    &IORegs::invalid_write,     // PPUDATA
};

data IORegs::read(address ptr)
{
    return (this->*m_read_clbks[ptr])();
}

void IORegs::write(address ptr, data value)
{
    (this->*m_write_clbks[ptr])(value);
}

void IORegs::oam_dma(uint8_t page)
{
    //printf("dma is being done\n");
    // disable cpu for 513 or 514 cycles
    if (m_cpu.cycles % 2) // odd
    {
        co_set_skip(m_cpu_co, 514);
    }
    else
    {
        co_set_skip(m_cpu_co, 513);
    }

    std::array<uint8_t, 256> data;
    for (size_t i { 0 }; i < 256; ++i)
    {
        data[i] = m_cpu.read(((uint16_t)page)*256 + i);
    }

    m_ppu.oam_dma(data);
}

void IORegs::input_write(uint8_t val)
{
    m_input.input_write(val);
}

uint8_t IORegs::input_read16()
{
    return m_input.output1_read();
}

uint8_t IORegs::input_read17()
{
    return m_input.output2_read();
}

uint8_t IORegs::invalid_read()
{
    return 0;
}

void IORegs::invalid_write(uint8_t val)
{
    (void)val;
}
