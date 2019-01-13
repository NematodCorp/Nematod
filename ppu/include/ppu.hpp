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

#include "memory.hpp"
#include "interrupts/include/interrupts.hpp"
#include "cpu.hpp" // for debug

class PPU : public InterruptEmitter<NMI>
{
    friend class PPUCtrlRegs;
    friend class IORegs;

public:
    cpu6502* cpu;
    AddressSpace addr_space;
    std::array<uint8_t, 240*256> framebuffer;
    unsigned frames { 0 };

    constexpr static std::array<uint32_t, 0x40> ppu_palette =
    {0x545454, 0x001E74, 0x081090, 0x300088, 0x440064, 0x5C0030, 0x540400, 0x3C1800, 0x202A00, 0x083A00, 0x004000, 0x003C00, 0x00323C, 0x000000, 0x000000, 0x000000,
     0x989698, 0x084CC4, 0x3032EC, 0x5C1EE4, 0x8814B0, 0xA01464, 0x982220, 0x783C00, 0x545A00, 0x287200, 0x087C00, 0x007628, 0x006678, 0x000000, 0x000000, 0x000000,
     0xECEEEC, 0x4C9AEC, 0x787CEC, 0xB062EC, 0xE454EC, 0xEC58B4, 0xEC6A64, 0xD48820, 0xA0AA00, 0x74C400, 0x4CD020, 0x38CC6C, 0x38B4CC, 0x3C3C3C, 0x000000, 0x000000,
     0xECEEEC, 0xA8CCEC, 0xBCBCEC, 0xD4B2EC, 0xECAEEC, 0xECAED4, 0xECB4B0, 0xE4C490, 0xCCD278, 0xB4DE78, 0xA8E290, 0x98E2B4, 0xA0D6E4, 0xA0A2A0, 0x000000, 0x000000, };


public:
    void power_up();
    void reset();

    void render_frame();

private:
    unsigned sprite_height() const
    { return (m_ctrl & SpriteSize8x16) ? 16 : 8; }

    uint8_t ppu_read(uint16_t addr)
    {
        uint8_t data = addr_space.read(addr);
        return data;
    }

    void sprite_evaluation();
    void prefetch_sprites();
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

    void copy_palette_data();

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
        IsSprite0       = (1<<3), // only used to mark a prefetech_sprite as the sprite 0, not an actual PPU flag
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

    uint8_t                     m_oam_addr { 0 };

    bool                        m_suppress_vbl { false };
    bool                        m_skip_cycle { false };
    unsigned                    m_sprite0_hit_cycle { UINT_MAX };
    unsigned                    m_sprite_overflow_cycle { UINT_MAX };

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
