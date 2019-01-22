/*
nes.cpp

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

#include "nes.hpp"

#include "clock.hpp"
#include "common/parallel_stepper.hpp"

#include "ppu/include/ppu.hpp"
#include "ppu/include/ppu_regs.hpp"
#include "cpu/include/cpu.hpp"
#include "cpu/include/io_regs.hpp"
#include "input/include/inputadapter.hpp"

namespace NES
{
namespace internal
{
static uint8_t cpu6502_read(uint16_t addr)
{
    return cpu_space.read(addr);
}
static void cpu6502_write(uint16_t addr, uint8_t val)
{
    cpu_space.write(addr, val);
}
}

size_t total_cycles;
size_t oam_decay_cycles;

RAM<0x0400> nt1, nt2, nt3, nt4;
InputAdapter input;
PPU     ppu;
PPUCtrlRegs ppu_regs { ppu };
cpu6502 cpu {internal::cpu6502_read, internal::cpu6502_write};
IORegs io_regs{cpu, nullptr, ppu, input};

AddressSpace cpu_space;

RAM<0x0800> nes_ram;
RAM<0x0100> palette_ram;

struct CPUReceiver : public DividedClockReceiver<12>
{
    void on_active_clock() override
    {
        cpu.run(1000);
    };
};
struct PPUReceiver : public DividedClockReceiver<4>
{
    void on_active_clock() override
    {
        ppu.render_frame();
    };
};

CPUReceiver cpu_rcv;
PPUReceiver ppu_rcv;

ParallelStepper stepper{cpu_rcv, ppu_rcv};

void init()
{
    oam_decay_cycles = total_cycles = 0;
    cpu_space.clear();
    ppu.addr_space.clear();

    ppu.cpu = &cpu;
    io_regs.m_cpu_co = stepper.m_coroutines[1].co.co;

    cpu_space.add_port(memory_port{&nes_ram , 0x0000}); // RAM mirroring
    cpu_space.add_port(memory_port{&nes_ram , 0x0800});
    cpu_space.add_port(memory_port{&nes_ram , 0x1000});
    cpu_space.add_port(memory_port{&nes_ram , 0x1800});
    cpu_space.add_port(memory_port{&ppu_regs, 0x2000});
    cpu_space.add_port(memory_port{&io_regs,  0x4000});

    ppu.addr_space.add_port(memory_port{&palette_ram, 0x3F00});
}

void soft_reset()
{
    total_cycles = 0;
    cpu.reset(); ppu_regs.reset(); ppu.reset();
    stepper.reset(); // reset coroutines state
}

void power_cycle()
{
    oam_decay_cycles = 0;
    soft_reset();
}

void run_frame()
{
    // run until vblank then draw frame to prevent rendering artifacts
    for (size_t i { 0 }; i < (341*4/12)*241; ++i) // cpu clocks per scanline * 241
    {
        run_cpu_cycle();
    }

    // run until scanline 241 is actually reached
    while (ppu.m_current_line != 241)
    {
        run_cpu_cycle();
    }
}

void run_cpu_cycle()
{
    stepper.step_whole();
    total_cycles += 12; oam_decay_cycles += 12;
    // handle oam data decay
    if (oam_decay_cycles >= 12886364) // 600 msec
    {
        ppu_regs.clear_decay();
        oam_decay_cycles = 0;
    }
}

void run_ppu_cycle()
{
    // FIXME : further step_whole() will not take in account cycle clocks taken here, fix
    stepper.step();
}

void set_mirroring(const mirroring_config &config)
{
    // clear existing mirroring configuration
    ppu.addr_space.remove_port(0x2000);
    ppu.addr_space.remove_port(0x2400);
    ppu.addr_space.remove_port(0x2800);
    ppu.addr_space.remove_port(0x2C00);

    ppu.addr_space.add_port(memory_port{&config.top_left, 0x2000});
    ppu.addr_space.add_port(memory_port{&config.top_right, 0x2400});
    ppu.addr_space.add_port(memory_port{&config.bottom_left, 0x2800});
    ppu.addr_space.add_port(memory_port{&config.bottom_right, 0x2C00});
}

}
