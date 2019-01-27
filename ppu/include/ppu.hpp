/*
ppu.hpp

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
#ifndef PPU_HPP
#define PPU_HPP

#include <cstdint>
#include <climits>
#include <array>

// TODO : implement peek-poke
// TODO : fix these ugly includes
#include "memory/include/memory.hpp"
#include "interrupts/include/interrupts.hpp"
#include "cpu/include/cpu.hpp" // for debug

class PPU : public InterruptEmitter<NMI>
{
    friend class PPUCtrlRegs;
    friend class IORegs;

public:
    cpu6502* cpu;
    AddressSpace addr_space;
    std::array<uint8_t, 240*256> framebuffer;
    unsigned frames { 0 };

    // ABGR format
    constexpr static std::array<uint32_t, 0x40> ppu_palette =
    { 0xff7c7c7c,
    0xfffc0000, 0xffbc0000, 0xffbc2844, 0xff840094, 0xff2000a8, 0xff0010a8, 0xff001488, 0xff003050,
    0xff007800, 0xff006800, 0xff005800, 0xff584000, 0xff000000, 0xff000000, 0xff000000, 0xffbcbcbc,
    0xfff87800, 0xfff85800, 0xfffc4468, 0xffcc00d8, 0xff5800e4, 0xff0038f8, 0xff105ce4, 0xff007cac,
    0xff00b800, 0xff00a800, 0xff44a800, 0xff888800, 0xff000000, 0xff000000, 0xff000000, 0xfff8f8f8,
    0xfffcbc3c, 0xfffc8868, 0xfff87898, 0xfff878f8, 0xff9858f8, 0xff5878f8, 0xff44a0fc, 0xff00b8f8,
    0xff18f8b8, 0xff54d858, 0xff98f858, 0xffd8e800, 0xff787878, 0xff000000, 0xff000000, 0xfffcfcfc,
    0xfffce4a4, 0xfff8b8b8, 0xfff8b8d8, 0xfff8b8f8, 0xffc0a4f8, 0xffb0d0f0, 0xffa8e0fc, 0xff78d8f8,
    0xff78f8d8, 0xffb8f8b8, 0xffd8f8b8, 0xfffcfc00, 0xfff8d8f8, 0xff000000, 0xff000000 };


public:
    void power_up();
    void reset();

    void render_frame();

private:
    unsigned sprite_height() const
    { return (m_ctrl & SpriteSize8x16) ? 16 : 8; }
    uint16_t vram_addr()
    {
        return m_v;
    }

    uint8_t ppu_read(uint16_t addr)
    {
        uint8_t data = addr_space.read(addr);
        return data;
    }

    void sprite_evaluation();
    void prefetch_sprites();
    void do_buggy_overflow_evaluation(uint8_t starting_sprite);
    void fetch_next_tile();
    void render_tile(unsigned tile_idx);
    void do_unused_nt_fetches();
    void reload_shifts();

    void set_vblank();
    void update_nmi_logic();

    enum ScanlineType
    {
        Render,
        PreRender,
        Idle
    };

    template <ScanlineType Type>
    void scanline();

private:
    void cycle(int amnt = 1);

    enum StatusFlags : uint8_t
    {
        SpriteOverflow = (1<<5),
        Sprite0Hit     = (1<<6),
        VerticalBlank  = (1<<7)
    };

    enum CtrlFlags   : uint8_t
    {
        VRAMIncrement32 = (1<<2),
        SpriteTableAddr = (1<<3),
        BackGrTableAddr = (1<<4),
        SpriteSize8x16  = (1<<5),
        PPUMasterSlave  = (1<<6),
        VBlankNMI       = (1<<7)
    };

    enum MaskFlags   : uint8_t
    {
        Greyscale       = (1<<0),
        ShowLeftmostBG  = (1<<1),
        ShowLeftmostOAM = (1<<2),
        ShowBG          = (1<<3),
        ShowOAM         = (1<<4),
        RedEmphasis     = (1<<5),
        GreenEmphasis   = (1<<6),
        BlueEmphasis    = (1<<7),
    };

    uint8_t oam_read(uint8_t addr)
    { return ((uint8_t*)(m_oam_memory.data()))[addr]; }
    void    oam_write(uint8_t addr, uint8_t val)
    { ((uint8_t*)(m_oam_memory.data()))[addr] = val; }

    void    oam_dma(const std::array<uint8_t, 256>& data);

    bool sprite_in_range(uint8_t y_pos) const
    { return m_current_line >= y_pos &&
                m_current_line < y_pos + sprite_height() &&
                y_pos != 255; }

    void reset_horizontal_scroll();
    void reset_vertical_scroll();
    void coarse_x_increment();
    void y_increment();

    void set_unread_flags();

    bool rendering_enabled() const
    { return (m_mask & ShowBG) || (m_mask & ShowOAM); }

private:
    enum OAMAttributes : uint8_t
    {
        Palette         = 0b11,
        Sprite0         = (1<<3), // not an actual NES flag
        BehindBG        = (1<<5),
        HorizontalFlip  = (1<<6),
        VerticalFlip    = (1<<7)
    };

    struct sprite_data
    {
        uint8_t y_pos;
        uint8_t tile_index;
        uint8_t attributes;
        uint8_t x_pos;
    };
    struct prefetched_sprite
    {
        uint8_t x_pos;
        uint8_t attributes;
        uint8_t pattern_high; // bit 1 of palette idx; 8 times
        uint8_t pattern_low;  // bit 0 of palette idx; 8 times
    };

public:
    // scrolling internal registers
    uint16_t                    m_v { 0 };
    uint16_t                    m_t { 0 };
    uint8_t                     m_x { 0 };

    uint16_t                    m_delayed_vram_addr { 0 };

    uint8_t                     m_oam_addr { 0 };

    bool                        m_suppress_vbl { false };
    bool                        m_skip_cycle { false };

    unsigned                    m_sprite0_hit_cycle { UINT_MAX };
    unsigned                    m_sprite_overflow_cycle { UINT_MAX };
    unsigned                    m_delayed_vram_cycle { UINT_MAX };

    uint16_t                    m_tile_bmp_lo;
    uint16_t                    m_tile_bmp_hi;

    uint8_t                     m_attr_shift_lo;
    uint8_t                     m_attr_shift_hi;
    uint8_t                     m_attr_latch_lo;
    uint8_t                     m_attr_latch_hi;

    uint8_t                     m_prefetched_at_lo;
    uint8_t                     m_prefetched_at_hi;
    uint8_t                     m_prefetched_bg_lo;
    uint8_t                     m_prefetched_bg_hi;

    bool                        m_odd_frame { false };
    uint8_t                     m_status { 0 };
    uint8_t                     m_ctrl { 0 };
    uint8_t                     m_mask { 0 };
    size_t                      m_current_line { 0 };
    unsigned                    m_clocks { 0 };
    std::array<sprite_data, 64> m_oam_memory;
    std::array<sprite_data, 8>  m_secondary_oam;

    std::array<prefetched_sprite, 8> m_prefetched_sprites;
    std::array<uint8_t, 0x20>        m_palette_copy;
};

#endif // PPU_HPP
