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

#include "memory.hpp"

class PPU
{
    friend class PPUCtrlRegs;

public:
    AddressSpace addr_space;

private:
    void set_ctrl(uint8_t val);
    void set_mask(uint8_t mask);

    bool in_vblank() const;
    bool sprite_0_hit() const;
    bool sprite_overflow() const;

    uint8_t read(uint8_t addr);
    void    write(uint8_t addr, uint8_t val);

    uint8_t oam_read(uint8_t addr);
    void    oam_write(uint8_t addr, uint8_t val);

    void    fine_scroll(uint8_t x, uint8_t y);
};

#endif // PPU_HPP
