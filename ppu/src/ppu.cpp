/*
ppu.cpp

Copyright (c) 17 Yann BOUCHER (yann)

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

#include "ppu.hpp"
#include "assert.h"

#include "common/coroutine.hpp"

enum VRegComponents : uint16_t
{
    CoarseX = 0x1F,
    CoarseY = 0x3E0,
    HNameTable = 0x400,
    VNameTable = 0x800,
    FineYScroll = 0x7000
};

constexpr std::array<uint32_t, 0x40> ppu_palette =
{0x545454, 0x001E74, 0x081090, 0x300088, 0x440064, 0x5C0030, 0x540400, 0x3C1800, 0x202A00, 0x083A00, 0x004000, 0x003C00, 0x00323C, 0x000000, 0x000000, 0x000000,
 0x989698, 0x084CC4, 0x3032EC, 0x5C1EE4, 0x8814B0, 0xA01464, 0x982220, 0x783C00, 0x545A00, 0x287200, 0x087C00, 0x007628, 0x006678, 0x000000, 0x000000, 0x000000,
 0xECEEEC, 0x4C9AEC, 0x787CEC, 0xB062EC, 0xE454EC, 0xEC58B4, 0xEC6A64, 0xD48820, 0xA0AA00, 0x74C400, 0x4CD020, 0x38CC6C, 0x38B4CC, 0x3C3C3C, 0x000000, 0x000000,
 0xECEEEC, 0xA8CCEC, 0xBCBCEC, 0xD4B2EC, 0xECAEEC, 0xECAED4, 0xECB4B0, 0xE4C490, 0xCCD278, 0xB4DE78, 0xA8E290, 0x98E2B4, 0xA0D6E4, 0xA0A2A0, 0x000000, 0x000000, };

void PPU::cycle(int amnt)
{
    for (int i { 0 }; i < amnt; ++i)
    {
        co_yield();
        ++m_clocks;
    }
}

void PPU::render_frame()
{
    m_current_line = 0;
    for (size_t i { 0 }; i < 240; ++i)
    {
        scanline<Render>();
    }
    for (size_t i { 240 }; i <= 260; ++i)
    {
        scanline<Idle>();
    }
    scanline<PreRender>();

    m_odd_frame ^= 1;
    ++frames;
}

template <PPU::ScanlineType Type>
void PPU::scanline()
{
    cycle(); // idle cycle

    if constexpr (Type != Idle)
    {
        if constexpr (Type == PreRender)
        {
            m_status &= (~(VerticalBlank | Sprite0Hit | SpriteOverflow)); // clear status flags
            update_nmi_logic();
        }

        sprite_evaluation();
        for (size_t i { 0 }; i < 256 / 8; ++i)
        {
            fetch_next_tile();
            if constexpr (Type == Render)
                    render_tile(i);
            coarse_x_increment();
        }
        reset_horizontal_scroll();
        prefetch_sprites(); // prefetch next scanline's sprites

        if constexpr (Type == PreRender)
        {
            reset_vertical_scroll();
        }

        fetch_next_tile();
        fetch_next_tile(); // fetch next two tiles

        do_unused_nt_fetches();
    }
    else
    {
        if (m_current_line == 241)
        {
            if (!m_suppress_vbl)
            {
                set_vblank();
            }
            else
            {
                m_suppress_vbl = false;
            }
        }
        cycle(340);
    }

    ++m_current_line;
    m_clocks = 0;
}

void PPU::render_tile(unsigned tile_idx)
{
    for (size_t i { 0 }; i < 8; ++i)
    {
        unsigned x_pos = tile_idx*8 + i;

        const bool render_bg      = m_ctrl & ShowBG  && !(x_pos < 8 && !(m_ctrl & ShowLeftmostBG ));
        const bool render_sprites = m_ctrl & ShowOAM && !(x_pos < 8 && !(m_ctrl & ShowLeftmostOAM));

        uint8_t bg_pattern = 0;
        uint8_t bg_palette = m_bg_palette;

        if (render_bg)
        {
            bg_pattern = ((m_tile_bmp_hi >> m_x)&0b1 >> 1) |
                    ((m_tile_bmp_lo >> m_x)&0b1);
        }

        uint8_t sprite_pattern = 0;
        uint8_t sprite_palette = 0;
        auto sprite = m_prefetched_sprites[i];

        if (render_sprites)
        {
            for (size_t i { 7 }; ; i--)
            {
                if (sprite.attributes == 0xFF) continue; // invalid
                if (x_pos < sprite.x_pos || x_pos >= sprite.x_pos + 8) continue;
                unsigned sprite_x = x_pos - sprite.x_pos;

                if (sprite.attributes & VerticalFlip)
                    sprite_x ^= 7;

                sprite_pattern = ((sprite.pattern_high << sprite_x)&0b1 >> 1) |
                        ((sprite.pattern_low  << sprite_x)&0b1     );
                sprite_palette = sprite.attributes & 0b11;

                // test sprite 0 hit
                if (render_bg && sprite_pattern && bg_pattern && x_pos != 255)
                {
                    m_status |= Sprite0Hit;
                }
            }
        }

        uint8_t output_color = 0;
        if (bg_pattern == 0)
        {
            if (sprite_pattern == 0)
            {
                output_color = ppu_read(0x3F00);
            }
            else
            {
                output_color = ppu_read(0x3F10 + sprite_palette*4 + sprite_pattern);
            }
        }
        else
        {
            if (sprite_pattern && !(sprite.attributes & BehindBG))
            {
                output_color = ppu_read(0x3F10 + sprite_palette*4 + sprite_pattern);
            }
            else
            {
                output_color = ppu_read(0x3F00 + bg_palette*4 + bg_pattern);
            }
        }

        if (m_ctrl & Greyscale)
        {
            output_color &= 0x30; // only use colors from the grey column of the NES palette
        }

        framebuffer[tile_idx*8 + i] = output_color;

        m_tile_bmp_lo >>= 1;
        m_tile_bmp_hi >>= 1;
    }
}

void PPU::do_unused_nt_fetches()
{
    constexpr uint16_t nt_base[4] = {0x2000, 0x2400, 0x2800, 0x2C00};
    uint16_t nt_addr = nt_base[m_ctrl&0b11] | (m_v & ~FineYScroll);

    ppu_read(nt_addr); cycle(2); // dummy NT read
    ppu_read(nt_addr); cycle(1); // dummy NT read
    if (m_current_line == 261 && m_odd_frame && rendering_enabled() && false)
    {
        cycle(0); // skip cycle on odd frames
    }
    else
    {
        cycle(1);
    }
}

void PPU::set_vblank()
{
    m_status |= VerticalBlank;
    update_nmi_logic();
}

void PPU::update_nmi_logic()
{
    if ((m_status & VerticalBlank) && (m_ctrl & VBlankNMI))
    {
        cpu->m_int_delay = true;
        cpu->pull_nmi_low();
    }
    else
    {
        cpu->pull_nmi_high();
    }
}

unsigned PPU::sprite_height() const
{
    return (m_ctrl & SpriteSize8x16) ? 16 : 8;
}

uint8_t PPU::ppu_read(uint16_t addr)
{
    uint8_t data = addr_space.read(addr);
    return data;
}

void PPU::sprite_evaluation()
{
    std::fill(m_secondary_oam.begin(), m_secondary_oam.end(), sprite_data{0xFF, 0xFF, 0xFF, 0xFF});
    // can actually be done in an cycle-inaccurate way
    // only sprite tile fetches on pre-render line
    size_t found_sprites = 0;
    for (size_t i { 0 }; i < 64; ++i)
    {
        if (m_oam_memory[i].y_pos >= m_current_line &&
                m_oam_memory[i].y_pos < m_current_line + sprite_height())
        {
            m_secondary_oam[found_sprites] = m_oam_memory[i];
            if (i == 0) // mark as sprite 0
                m_secondary_oam[found_sprites].attributes |=  IsSprite0;
            else
                m_secondary_oam[found_sprites].attributes &= ~IsSprite0;
            ++found_sprites;
        }
        if (found_sprites == 8)
        {
            m_status |= SpriteOverflow;
            break;
        }
    }
}

void PPU::prefetch_sprites()
{
    constexpr uint16_t nt_base[4] = {0x2000, 0x2400, 0x2800, 0x2C00};
    uint16_t nt_addr = nt_base[m_ctrl&0b11] | (m_v & ~FineYScroll);
    // coarse scrolling registers are divided by 4 because attribute tables indexes 4x4 tile blocks
    /*               */ /*base*/ /* nametable */  /*  coarse y / 4 */   /*  coarse x / 4 */
    uint16_t attr_addr = 0x23C0 | (m_v & 0x0C00) | ((m_v >> 4) & 0x38) | ((m_v >> 2) & 0x07);

    for (size_t i { 0 }; i < 8; ++i)
    {
        auto data = m_secondary_oam[i];
        uint16_t addr = 0;
        // 8x16 sprites have a different addressing mode
        if (sprite_height() == 16)
            addr = ((data.tile_index & 1) * 0x1000) + ((data.tile_index & ~1) * 16);
        else
            addr = (m_ctrl&SpriteTableAddr ? 0x1000 : 0x0000) + (data.tile_index * 16);

        /*       dummy      */ ppu_read(nt_addr  ); cycle(2); // dummy NT read
        /*       dummy      */ ppu_read(attr_addr); cycle(2); // dummy AY read
        uint8_t pattern_low  = ppu_read(addr + 0 ); cycle(2);
        uint8_t pattern_high = ppu_read(addr + 8 ); cycle(2);

        if (data.y_pos >= 0xEF)
        {
            m_prefetched_sprites[i].attributes = 0xFF; // mark as invalid
        }
        else
        {
            m_prefetched_sprites[i].attributes = data.attributes;

            unsigned sprY = m_current_line - data.y_pos;  // Line inside the sprite.
            if (data.attributes & 0x80) sprY ^= sprite_height() - 1;      // Vertical flip.
            addr += sprY + (sprY & 8);  // Select the second tile if on 8x16.

            m_prefetched_sprites[i].pattern_low  = pattern_low;
            m_prefetched_sprites[i].pattern_high = pattern_high;
        }
    }
}

void PPU::fetch_next_tile()
{
    constexpr uint16_t nt_base[4] = {0x2000, 0x2400, 0x2800, 0x2C00};
    constexpr uint16_t bg_base[2] = {0x0000, 0x1000};

    uint16_t nt_addr = nt_base[m_ctrl&0b11] | (m_v & ~FineYScroll);
    // coarse scrolling registers are divided by 4 because attribute tables indexes 4x4 tile blocks
    /*               */ /*base*/ /* nametable */  /*  coarse  y/4  */   /*  coarse  x/4  */
    uint16_t attr_addr = 0x23C0 | (m_v & 0x0C00) | ((m_v >> 4) & 0x38) | ((m_v >> 2) & 0x07);

    uint8_t nt_byte   = ppu_read(nt_addr  ); cycle(2);
    uint8_t attr_byte = ppu_read(attr_addr); cycle(2);

    uint16_t pattern_addr = bg_base[(m_ctrl>>4)&0b1] + nt_byte + ((m_v & FineYScroll) >> 12);

    m_tile_bmp_lo >>= 8;
    m_tile_bmp_hi >>= 8;
    m_tile_bmp_lo |= ppu_read(pattern_addr  ) << 8; cycle(2);
    m_tile_bmp_hi |= ppu_read(pattern_addr+8) << 8; cycle(2);

    int coarse_x = m_v & CoarseX;
    int coarse_y = (m_v & CoarseY) >> 5;
    int attr_shift = ((coarse_y&0b10) + (coarse_x&0b10 >> 1))*2;

    m_bg_palette = (attr_byte >> attr_shift) & 0b11;
}

void PPU::oam_dma(const std::array<uint8_t, 256> &data)
{
    memcpy(m_secondary_oam.data(), data.data(), 256);
}

void PPU::reset_horizontal_scroll()
{
    // reset the horizontal components of v
    m_v &= ~CoarseX; m_v &= ~HNameTable;
    m_v |= (m_t & (HNameTable | CoarseX));
}

void PPU::reset_vertical_scroll()
{
    // reset the vertical components of v
    m_v &= ~CoarseY; m_v &= ~VNameTable; m_v &= ~FineYScroll;
    m_v |= (m_t & (VNameTable | CoarseY | FineYScroll));
}

void PPU::coarse_x_increment()
{
    if ((m_v & CoarseX) != 31) // no overflow
    {
        m_v++; // increment CoarseX
    }
    else // overflow
    {
        m_v &= ~CoarseX; // set coarse x scroll to zero
        m_v ^= HNameTable; // flip horizontal nametable
    }
}

void PPU::y_increment()
{
    if ((m_v & FineYScroll) != 0x7000) // no overflow
    {
        m_v += 0x1000; // increment fine y scroll
    }
    else // overflow
    {
        m_v &= ~FineYScroll; // set fine y scroll to 0
        uint8_t coarse_y = (m_v & CoarseY) >> 5;
        if (coarse_y == 29) // overflow as row 29 is the last row of tiles
        {
            coarse_y = 0;
            m_v ^= VNameTable; // flip vertical nametable
        }
        else if (coarse_y == 31) // the user code set coarse_y to an invalid value; ignore flips
        {
            coarse_y = 0;
        }
        else
        {
            ++coarse_y;
        }
        m_v &= ~CoarseY;
        m_v |= (uint16_t)coarse_y << 5;
    }
}
