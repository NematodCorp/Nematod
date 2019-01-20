/*
nes.hpp

Copyright (c) 19 Yann BOUCHER (yann)

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
#ifndef NES_HPP
#define NES_HPP

#include "memory/include/memory.hpp"

class InputAdapter;
class PPU;
class PPUCtrlRegs;
class cpu6502;
class IORegs;

namespace NES
{
void init();

void soft_reset();
void power_cycle();

void run_frame();
void run_cpu_cycle();
void run_ppu_cycle();

// nametables
extern RAM<0x0400> nt1, nt2, nt3, nt4;
struct mirroring_config
{
    RAM<0x0400>& top_left   , & top_right;
    RAM<0x0400>& bottom_left, & bottom_right;
};

inline mirroring_config horizontal { nt1, nt1, nt2, nt2 };
inline mirroring_config vertical   { nt1, nt2, nt1, nt2 };
inline mirroring_config fourscreen { nt1, nt2, nt3, nt4 };
void set_mirroring(const mirroring_config& config);

extern InputAdapter input;
extern PPU     ppu;
extern PPUCtrlRegs ppu_regs;
extern cpu6502 cpu;
extern IORegs io_regs;

extern AddressSpace cpu_space;

extern RAM<0x0800> nes_ram;
extern RAM<0x0100> palette_ram;

}

#endif // NES_HPP
