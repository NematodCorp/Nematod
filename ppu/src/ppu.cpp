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

// in order to force inlining
#define cycle(amnt) \
    co_set_skip((amnt) - 1); \
    co_yield(); \
    m_clocks += (amnt);

void PPU::render_frame()
{
    scanline<PreRender>();
    m_current_line = 0;
    for (size_t i { 0 }; i < 240; ++i)
    {
        scanline<Render>();
    }
    for (size_t i { 240 }; i <= 260; ++i)
    {
        scanline<Idle>();
    }

    m_odd_frame ^= 1;
    ++frames;
}

void PPU::power_up()
{
    m_ctrl = 0;
    m_mask = 0;
    m_status = 0;
    m_oam_addr = 0;
    m_v = m_t = 0;
    m_odd_frame = false;
}

void PPU::reset()
{
    m_ctrl = 0;
    m_mask = 0;
    m_v = m_t = 0;
    m_odd_frame = false;
}

template <PPU::ScanlineType Type>
void PPU::scanline()
{
        //printf("scanline %d : addr : m_v : 0x%x\n", m_current_line, m_v&0x0C00);
    if constexpr (Type == PreRender)
    {
        m_status &= (~(Sprite0Hit)); // clear sprite 0 flag on first cycle (why ? no idea, but passes timing tests)
        m_sprite0_hit_cycle = UINT_MAX;

        m_status &= (~(SpriteOverflow)); // clear sprite 0 flag on first cycle (why ? no idea, but passes timing tests)
        m_sprite_overflow_cycle = UINT_MAX;

        //update_nmi_logic();
    }

    if (!m_skip_cycle)
    {
        cycle(1); // idle cycle
    }

    if constexpr (Type != Idle)
    {
        if constexpr (Type == PreRender)
        {
            m_status &= (~(VerticalBlank)); // clear status flags
            update_nmi_logic();
        }
        if (rendering_enabled())
            sprite_evaluation();

        for (size_t i { 0 }; i < 256 / 8; ++i)
        {
            if constexpr (Type == Render)
                    render_tile(i);
            fetch_next_tile(); reload_shifts();
        }

        if (rendering_enabled())
        {
            y_increment(); // actually 1 cycle late, may cause problems ?
            reset_horizontal_scroll();
        }
        prefetch_sprites(); // prefetch next scanline's sprites

        if constexpr (Type == PreRender)
        {
            if (rendering_enabled()) reset_vertical_scroll();
        }

        // fetch next two tiles
        fetch_next_tile(); reload_shifts();
        m_tile_bmp_lo <<= 8;
        m_tile_bmp_hi <<= 8;
        for (size_t i { 0 }; i < 8; ++i)
        {
            m_attr_shift_lo <<= 1; m_attr_shift_lo |= m_attr_latch_lo;
            m_attr_shift_hi <<= 1; m_attr_shift_hi |= m_attr_latch_hi;
        }
        fetch_next_tile(); reload_shifts();


        do_unused_nt_fetches();
        // copy palette data so render_tile doesn't make calls to ppu_read for each pixel of the scanline, results in less overhead
        // prevents palette modification during scanline but isn't much of a problem since palette change mid-scaline is virtually impossible
        copy_palette_data();
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
        cycle(1);
        cycle(1); // explicit cycling so we can handle NMI suppression
        cycle(338);
    }

    ++m_current_line;
    m_clocks = 0;

    // ensure that sprite0 and sprite_overflow flags are set even in $2002 wasn't read during the frame
    set_unread_flags();
}

void PPU::render_tile(unsigned tile_idx)
{
    const bool render_bg      = (m_mask & ShowBG ) && !(tile_idx == 0 && !(m_mask & ShowLeftmostBG ));
    const bool render_sprites = (m_mask & ShowOAM) && !(tile_idx == 0 && !(m_mask & ShowLeftmostOAM));


    for (size_t i { 0 }; i < 8; ++i)
    {
        unsigned x_pos = tile_idx*8 + i;

        uint8_t bg_pattern = 0;
        uint8_t bg_palette = 0;

        if (render_bg)
        {
            bg_pattern = (((m_tile_bmp_hi >> (15-m_x))&0b1) << 1) |
                    ((m_tile_bmp_lo >> (15-m_x))&0b1);
            bg_palette = (((m_attr_shift_hi >> (7-m_x))&0b1) << 1) |
                    ((m_attr_shift_lo >> (7-m_x))&0b1);
        }

        uint8_t sprite_pattern = 0;
        uint8_t sprite_palette = 0;

        prefetched_sprite sprite;
        if (render_sprites)
        {
            for (size_t j { 0 }; j<8;++j)
            {
                auto temp_sprite = m_prefetched_sprites[j];

                if (temp_sprite.attributes == 0xFF) continue; // invalid
                if (x_pos < temp_sprite.x_pos || x_pos >= (unsigned)(temp_sprite.x_pos) + 8) continue;
                sprite = temp_sprite;

                unsigned sprite_x = x_pos - sprite.x_pos;

                if (sprite.attributes & HorizontalFlip)
                    sprite_x ^= 7;

                uint8_t pattern = (((sprite.pattern_high >> (7-sprite_x))&0b1) << 1) |
                        ((sprite.pattern_low  >> (7-sprite_x))&0b1     );
                if (pattern == 0) continue;

                sprite_pattern = pattern;
                sprite_palette = sprite.attributes & 0b11;

                // test sprite 0 hit
                if (j == 0 && m_sprite0_on_scanline && sprite_pattern && bg_pattern && x_pos != 255)
                {
                    if (m_sprite0_hit_cycle == UINT_MAX)
                        m_sprite0_hit_cycle = x_pos+1;
                }
                break;
            }
        }

        uint8_t output_color = 0;
        if (bg_pattern == 0)
        {
            if (sprite_pattern == 0)
            {
                output_color = m_palette_copy[0x00];
            }
            else
            {
                output_color = m_palette_copy[0x10 | sprite_palette*4 | sprite_pattern];
            }
        }
        else
        {
            if (sprite_pattern && !(sprite.attributes & BehindBG))
            {
                output_color = m_palette_copy[0x10 | sprite_palette*4 | sprite_pattern];
            }
            else
            {
                output_color = m_palette_copy[0x00 | bg_palette*4 | bg_pattern];
            }
        }

        if (m_mask & Greyscale)
        {
            output_color &= 0x30; // only use colors from the grey column of the NES palette
        }

        framebuffer[x_pos + m_current_line*256] = output_color;
        // TODO : emphasis bits

        m_tile_bmp_lo <<= 1;
        m_tile_bmp_hi <<= 1;
        m_attr_shift_lo <<= 1; m_attr_shift_lo |= m_attr_latch_lo;
        m_attr_shift_hi <<= 1; m_attr_shift_hi |= m_attr_latch_hi;
    }
}

void PPU::do_unused_nt_fetches()
{
    uint16_t nt_addr = 0x2000 | (m_v & ~FineYScroll);

    ppu_read(nt_addr); cycle(1); // dummy NT read

    if (m_current_line == 261 && m_odd_frame && rendering_enabled())
        m_skip_cycle = true;
    else
        m_skip_cycle = false;

    cycle(1);
    ppu_read(nt_addr); cycle(2); // dummy NT read
}

void PPU::reload_shifts()
{
    m_tile_bmp_lo &= 0xFF00;
    m_tile_bmp_hi &= 0xFF00;

    m_tile_bmp_lo |= m_prefetched_bg_lo;
    m_tile_bmp_hi |= m_prefetched_bg_hi;

    m_attr_latch_lo = m_prefetched_at_lo;
    m_attr_latch_hi = m_prefetched_at_hi;
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

void PPU::sprite_evaluation()
{
    std::fill(m_secondary_oam.begin(), m_secondary_oam.end(), sprite_data{0xFF, 0xFF, 0xFF, 0xFF});
    // can actually be done in an cycle-inaccurate way
    // only sprite tile fetches on pre-render line
    size_t found_sprites = 0;
    for (size_t i { 0 }; i < 64; ++i)
    {
        if (m_current_line >= m_oam_memory[i].y_pos &&
                m_current_line < m_oam_memory[i].y_pos + sprite_height() &&
                m_oam_memory[i].y_pos != 255)
        {
            if (found_sprites == 8)
            {
                int count = 65 + 49 + (i-1)*2 + rendering_enabled();
                //printf("with clocks %d, i %d\n", count, m_current_line);
                if (m_sprite_overflow_cycle > count)
                    m_sprite_overflow_cycle = count;
                //printf("sprite overflow on scanline %d\n", m_current_line);
                break;
            }

            m_secondary_oam[found_sprites] = m_oam_memory[i];
            if (i == 0) // mark as sprite 0
                m_sprite0_on_scanline = true;
            else
                m_sprite0_on_scanline = false;
            ++found_sprites;
        }
    }
}

void PPU::prefetch_sprites()
{
    uint16_t nt_addr = 0x2000 | (m_v & ~FineYScroll);
    // coarse scrolling registers are divided by 4 because attribute tables indexes 4x4 tile blocks
    /*               */ /*base*/ /* nametable */  /*  coarse y / 4 */   /*  coarse x / 4 */
    uint16_t attr_addr = 0x23C0 | (m_v & 0x0C00) | ((m_v >> 4) & 0x38) | ((m_v >> 2) & 0x07);

    for (size_t i { 0 }; i < 8; ++i)
    {
        auto data = m_secondary_oam[i];
        uint16_t addr;
        // 8x16 sprites have a different addressing mode
        if (sprite_height() == 16)
            addr = ((data.tile_index & 0b1) * 0x1000) + ((data.tile_index & ~0b1) * 0x10);
        else
            addr = ((m_ctrl&SpriteTableAddr) ? 0x1000 : 0x0000) + (data.tile_index * 0x10);

        unsigned sprY = m_current_line - data.y_pos;  // Line inside the sprite.
        if (data.attributes & VerticalFlip) sprY ^= sprite_height() - 1;      // Vertical flip.
        addr += sprY + (sprY & 8);  // Select the second tile if on 8x16.

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
            m_prefetched_sprites[i].x_pos = data.x_pos;

            m_prefetched_sprites[i].pattern_low  = pattern_low;
            m_prefetched_sprites[i].pattern_high = pattern_high;
        }
    }
}

void PPU::fetch_next_tile()
{
    constexpr uint16_t bg_base[2] = {0x0000, 0x1000};

    int coarse_x = m_v & CoarseX;
    int coarse_y = (m_v & CoarseY) >> 5;

    uint16_t nt_addr = 0x2000 | (m_v & ~FineYScroll);
    // coarse scrolling registers are divided by 4 because attribute tables indexes 4x4 tile blocks
    /*               */ /*base*/ /* nametable */  /*  coarse  y/4  */   /*  coarse  x/4  */
    uint16_t attr_addr = 0x23C0 | (m_v & 0x0C00) | ((m_v >> 4) & 0x38) | ((m_v >> 2) & 0x07);
    //uint16_t attr_addr = 0x23C0 | (m_v & 0x0C00) | ((coarse_y/4)<<3) | (coarse_x/4);

    uint8_t nt_byte   = ppu_read(nt_addr  ); cycle(2);
    uint8_t attr_byte = ppu_read(attr_addr); cycle(2);

    uint16_t pattern_addr = nt_byte*0x10 + bg_base[(m_ctrl>>4)&0b1] + ((m_v & FineYScroll) >> 12);

    m_prefetched_bg_lo = ppu_read(pattern_addr  ); cycle(2);
    m_prefetched_bg_hi = ppu_read(pattern_addr+8); cycle(2);

    if (coarse_y & 0b10)
        attr_byte >>= 4;
    if (coarse_x & 0b10)
        attr_byte >>= 2;

    m_prefetched_at_lo = attr_byte & 0b1;
    m_prefetched_at_hi = (attr_byte & 0b10) >> 1;

    if (rendering_enabled())
    {
        coarse_x_increment();
    }
}

void PPU::oam_dma(const std::array<uint8_t, 256> &data)
{
    for (size_t i { 0 }; i < 256; ++i)
    {
        uint8_t addr = m_oam_addr + i;
        oam_write(addr, data[i]);
    }
}

void PPU::copy_palette_data()
{
    for (size_t i { 0 }; i < 0x20; ++i)
    {
        m_palette_copy[i] = ppu_read(0x3F00 + i);
    }
}

void PPU::reset_horizontal_scroll()
{
    // reset the horizontal components of v
    m_v &= ~(HNameTable | CoarseX);
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
        uint16_t coarse_y = (m_v & CoarseY) >> 5;
        if (coarse_y == 29) // overflow as row 29 is the last row of tiles
        {
            coarse_y = 0;
            m_v ^= VNameTable; // flip vertical nametable
        }
        else if (coarse_y == 31) // the user code set coarse_y to an invalid value; ignore flips
        {
            printf("Coarse Y was set to an invalid value\n"); // FIXME : log
            coarse_y = 0;
        }
        else
        {
            ++coarse_y;
        }
        m_v &= ~CoarseY;
        m_v |= (coarse_y << 5);
    }
}

void PPU::set_unread_flags()
{
    // ensure they are immediately set
    if (m_sprite0_hit_cycle != UINT_MAX)
    {
        m_sprite0_hit_cycle = 0;
    }
    if (m_sprite_overflow_cycle != UINT_MAX)
    {
        m_sprite_overflow_cycle = 0;
    }
}
