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

void PPUCtrlRegs::clear_decay()
{
    m_decay = 0;
}

void PPUCtrlRegs::reset()
{
    m_decay = 0;
    m_w = false;
    m_read_buffer = 0;
}

data PPUCtrlRegs::read(address ptr)
{
    return (this->*m_read_clbks[ptr % 8])();
}

void PPUCtrlRegs::write(address ptr, data value)
{
    m_decay = value;

    (this->*m_write_clbks[ptr % 8])(value);
}

uint8_t PPUCtrlRegs::status_read()
{
    m_w = false; // clear w

    uint8_t status = (m_ppu.m_status & 0x80) | (m_decay & 0x1F); // ignore Sprite0 and overflow for now, lower 5 bits should be == to decay

    if (m_ppu.m_clocks >= m_ppu.m_sprite0_hit_cycle)
    {
        status |= PPU::Sprite0Hit;
    }
    if (m_ppu.m_clocks >= m_ppu.m_sprite_overflow_cycle)
    {
        status |= PPU::SpriteOverflow;
    }

    // handle suppression behavior
    if (m_ppu.m_current_line == 241)
    {
        if (m_ppu.m_clocks == 0 )
        {
            m_ppu.m_suppress_vbl = true;
        }
        if (m_ppu.m_clocks == 1 || m_ppu.m_clocks == 2)
        {
            // don't generate NMI either
            // TODO : find a better way to express this
            m_ppu.cpu->m_nmi_pending = false;
        }
    }

    m_ppu.m_status &= ~PPU::VerticalBlank; // reset vblank flag

    m_ppu.update_nmi_logic();
    return status;
}

void PPUCtrlRegs::oam_addr_write(uint8_t val)
{
    m_ppu.m_oam_addr = val;
}

uint8_t PPUCtrlRegs::oam_data_read()
{
    uint8_t value = m_ppu.oam_read(m_ppu.m_oam_addr);

    if ((m_ppu.m_oam_addr % 4) == 2)
    {
        value &= 0xE3; // clear bits 2-4
        m_decay = value; // refresh decay during reads of the third byte of a sprite
    }

    return value;
}

void PPUCtrlRegs::oam_data_write(uint8_t val)
{
    if (m_ppu.rendering_enabled() && (m_ppu.m_current_line < 240 || m_ppu.m_current_line == 261))
    {
        val = 0xFF;
    }
    if ((m_ppu.m_oam_addr % 4) == 2) // why ? honestly no idea
    {
        val &= 0xE3;
    }

    m_ppu.oam_write(m_ppu.m_oam_addr, val);
    ++m_ppu.m_oam_addr; // writes increment OAMADDR
}

void PPUCtrlRegs::scroll_write(uint8_t val)
{
    if (!m_w)
    {
        m_ppu.m_t &= ~0b00000000'00011111;
                m_ppu.m_t |= (uint16_t)(val)>>3;

        m_ppu.m_x = val&0b111;
        m_w = true;
    }
    else
    {
        m_ppu.m_t &= ~0b1110011'11100000;
                m_ppu.m_t |= ((uint16_t)(val&0b111))<<12;
        m_ppu.m_t |= ((uint16_t)(val&0b11111000))<<2;

        m_w = false;
    }
}

void PPUCtrlRegs::addr_write(uint8_t val)
{
    if (!m_w)
    {
        m_ppu.m_t &= 0b00000000'11111111;
        m_ppu.m_t |= ((uint16_t)(val&0b0111111))<<8;

        m_w = true;
    }
    else
    {
        m_ppu.m_t &= 0b11111111'00000000;
                m_ppu.m_t |= val;
        m_ppu.m_delayed_vram_addr = m_ppu.m_t;
        m_ppu.m_delayed_vram_cycle = m_ppu.m_clocks+2;
        m_ppu.m_v = m_ppu.m_t;

        m_w = false;
    }
}

void PPUCtrlRegs::data_write(uint8_t val)
{
    uint16_t address = m_ppu.m_v&0x3FFF;
    if (address >= 0x3F00) // palette ram
    {
        uint8_t palette_index = address&0xFF;
        palette_index %= 0x20; // mirror addresses above 0x3F20
        if (palette_index % 4 == 0)
            palette_index %= 0x10; // mirror backdrop colors
        address&=0xFF00; address |= palette_index;

        // directly set palette contents so we don't have to call read() multiple times on each call to render_tile()
        m_ppu.m_palette_copy[palette_index] = val;
    }
    else if ((address&0xF000) == 0x3000) // do 0x3000-0x3EFF mirroring

    {
        address -= 0x1000;
    }

    m_ppu.addr_space.write(address, val);

    if (m_ppu.m_ctrl & PPU::VRAMIncrement32)
    {
        m_ppu.m_v += 32;
    }
    else
    {
        m_ppu.m_v++;
    }
}

uint8_t PPUCtrlRegs::data_read()
{
    uint8_t value;

    uint16_t address = m_ppu.m_v&0x3FFF;
    if (address >= 0x3F00) // palette ram
    {
        uint8_t palette_index = address&0xFF;
        palette_index %= 0x20; // mirror addresses above 0x3F20
        if (palette_index % 4 == 0)
            palette_index %= 0x10; // mirror backdrop colors
        address&=0xFF00; address |= palette_index;

        value = m_ppu.addr_space.read(address);
        m_read_buffer = m_ppu.addr_space.read((m_ppu.m_v & 0x2FFF)); // read nt data into read buffer, unaffected by mirroring

        // set 2 topmost bits of value to decay
        value &= 0x3F;
        value |= (m_decay & ~0x3F);

        m_decay &= 0x3F;
        m_decay |= value;
    }
    else
    {
        if ((address&0xF000) == 0x3000) // do 0x3000-0x3EFF mirroring
        {
            address -= 0x1000;
        }

        value = m_read_buffer;
        m_read_buffer = m_ppu.addr_space.read(address);

        m_decay = value;
    }

    if (m_ppu.m_ctrl & PPU::VRAMIncrement32)
    {
        m_ppu.m_v += 32;
    }
    else
    {
        m_ppu.m_v++;
    }

    return value;
}

void PPUCtrlRegs::ctrl_write(uint8_t val)
{
    m_ppu.m_ctrl = val;
    m_ppu.m_t   &= ~0b0001100'00000000;
            m_ppu.m_t   |= (val&0b11) << 10; // set base scrolling nametable

    if (m_ppu.m_clocks > 1) // passes nmi_on_timing.nes
        m_ppu.update_nmi_logic();
}

void PPUCtrlRegs::mask_write(uint8_t val)
{
    m_ppu.m_mask = val;
}

uint8_t PPUCtrlRegs::invalid_read()
{
    return m_decay;
}

void PPUCtrlRegs::invalid_write(uint8_t val)
{
    m_decay = val;
}
