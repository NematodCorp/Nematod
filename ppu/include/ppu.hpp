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

#include <array>

#include "memory.hpp"
#include "interrupts/include/interrupts.hpp"
#include "cpu.hpp" // for debug

class PPU : public InterruptEmitter<NMI>
{
    friend class PPUCtrlRegs;

public:
    cpu6502* cpu;
    AddressSpace addr_space;
    std::array<uint8_t, 240*256> framebuffer;
    unsigned frames { 0 };

public:
    void render_frame();

private:
    unsigned sprite_height() const;

    uint8_t ppu_read(uint16_t addr);

    void sprite_evaluation();
    void prefetch_sprites();
    void fetch_next_tile();
    void render_tile(unsigned tile_idx);
    void do_unused_nt_fetches();

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

    void reset_horizontal_scroll();
    void reset_vertical_scroll();
    void coarse_x_increment();
    void y_increment();

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

private:
    // scrolling internal registers
    uint16_t                    m_v { 0 };
    uint16_t                    m_t { 0 };
    uint8_t                     m_x { 0 };

    bool                        m_suppress_vbl { false };

    uint16_t                    m_tile_bmp_lo;
    uint16_t                    m_tile_bmp_hi;

    uint8_t                     m_bg_palette;

    bool                        m_odd_frame { false };
    uint8_t                     m_status { 0 };
    uint8_t                     m_ctrl { 0 };
    uint8_t                     m_mask { 0 };
    size_t                      m_current_line { 0 };
    unsigned                    m_clocks { 0 };
    std::array<sprite_data, 64> m_oam_memory;
    std::array<sprite_data, 8>  m_secondary_oam;

    std::array<prefetched_sprite, 8> m_prefetched_sprites;
};

#endif // PPU_HPP
