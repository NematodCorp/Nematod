/*
ppu_regs.hpp

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
#ifndef PPU_REGS_HPP
#define PPU_REGS_HPP

#include <array>

#include "memory/include/memory.hpp"

class PPU;

class PPUCtrlRegs : public MemoryInterfaceable
{
public:
    enum regs : uint8_t
    {
        PPUCTRL = 0x0,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROLL,
        PPUADDR,
        PPUDATA
    };

    PPUCtrlRegs(PPU& ppu) : MemoryInterfaceable(0x2000), m_ppu(ppu)
    {}

public:
    void clear_decay();
    void reset();

protected:
    data  read(address ptr)             override;
    void write(address ptr, data value) override;

private:
    uint8_t status_read();
    uint8_t oam_data_read();
    uint8_t data_read();

    void    ctrl_write(uint8_t val);
    void    mask_write(uint8_t val);
    void    oam_addr_write(uint8_t val);
    void    oam_data_write(uint8_t val);
    void    scroll_write(uint8_t val);
    void    addr_write(uint8_t val);
    void    data_write(uint8_t val);

    uint8_t invalid_read();
    void    invalid_write(uint8_t val);

private:
    using read_callback  = uint8_t(PPUCtrlRegs::*)();
    using write_callback = void(PPUCtrlRegs::*)(uint8_t);

    static std::array<read_callback, 8> m_read_clbks;
    static std::array<write_callback, 8> m_write_clbks;

    PPU& m_ppu;
    uint8_t m_decay = 0;
    uint8_t m_read_buffer = 0;

    bool m_w = false;
};

#endif // PPU_REGS_HPP
