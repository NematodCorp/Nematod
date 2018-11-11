/*
ppu_regs.cpp

Copyright (c) 11 Yann BOUCHER (yann)

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

#include "ppu_regs.hpp"
#include "ppu.hpp"

#include "common/bitops.hpp"

#include <cassert>

std::array<PPUCtrlRegs::read_callback, 8> PPUCtrlRegs::m_read_clbks =
{
    &PPUCtrlRegs::invalid_read, // PPUCTRL
    &PPUCtrlRegs::invalid_read, // PPUMASK
    &PPUCtrlRegs::status_read , // PPUSTATUS
    &PPUCtrlRegs::invalid_read, // OAMADDR
    &PPUCtrlRegs::oam_data_read,// OAMDATA
    &PPUCtrlRegs::invalid_read, // PPUSCROLL
    &PPUCtrlRegs::invalid_read, // PPUADDR
    &PPUCtrlRegs::data_read,    // PPUDATA
};
std::array<PPUCtrlRegs::write_callback, 8> PPUCtrlRegs::m_write_clbks =
{
    &PPUCtrlRegs::ctrl_write,     // PPUCTRL
    &PPUCtrlRegs::mask_write,     // PPUMASK
    &PPUCtrlRegs::invalid_write , // PPUSTATUS
    &PPUCtrlRegs::oam_addr_write, // OAMADDR
    &PPUCtrlRegs::oam_data_write, // OAMDATA
    &PPUCtrlRegs::scroll_write,   // PPUSCROLL
    &PPUCtrlRegs::addr_write,     // PPUADDR
    &PPUCtrlRegs::data_write,     // PPUDATA
};

data PPUCtrlRegs::read(address ptr)
{
    assert(ptr < 8);
    return (this->*m_read_clbks[ptr])();
}

void PPUCtrlRegs::write(address ptr, data value)
{
    assert(ptr < 8);
    (this->*m_write_clbks[ptr])(value);
}

uint8_t PPUCtrlRegs::status_read()
{
    uint8_t byte = 0;

    bit_change(byte, m_ppu.in_vblank(),       7);
    bit_change(byte, m_ppu.sprite_0_hit(),    6);
    bit_change(byte, m_ppu.sprite_overflow(), 5);

    return byte;
}

uint8_t PPUCtrlRegs::oam_data_read()
{
    return m_ppu.oam_read(m_oam_addr);
}

void PPUCtrlRegs::oam_addr_write(uint8_t val)
{
    m_oam_addr = val;
}

void PPUCtrlRegs::oam_data_write(uint8_t val)
{
    m_ppu.oam_write(m_oam_addr, val);
    ++m_oam_addr; // writes increment OAMADDR
}

void PPUCtrlRegs::scroll_write(uint8_t val)
{
    if (!m_mid_scroll_write)
    {
        m_scroll_x = val;
        m_mid_scroll_write = true;
    }
    else
    {
        uint8_t scroll_y = val;
        m_ppu.fine_scroll(m_scroll_x, scroll_y);
        m_mid_scroll_write = false;
    }
}

void PPUCtrlRegs::addr_write(uint8_t val)
{
    if (!m_mid_addr_write)
    {
        m_addr = (val << 8); // upper byte first
        m_mid_addr_write = true;
    }
    else
    {
        m_addr |= val;
        m_mid_addr_write = false;
    }
}

void PPUCtrlRegs::data_write(uint8_t val)
{
    m_ppu.write(m_addr, val);
}

uint8_t PPUCtrlRegs::data_read()
{
    return m_ppu.read(m_addr);
}

void PPUCtrlRegs::ctrl_write(uint8_t val)
{
    m_ppu.set_ctrl(val);
}

void PPUCtrlRegs::mask_write(uint8_t val)
{
    m_ppu.set_mask(val);
}

uint8_t PPUCtrlRegs::invalid_read()
{
    assert(false); // TODO : log
}

void PPUCtrlRegs::invalid_write(uint8_t val)
{
    assert(false); // TODO : log
}
