/*
functional_test.cpp

Copyright (c) 07 Yann BOUCHER (yann)

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

#include "gtest/gtest.h"

#include <cstdio>

#include "cpu.hpp"

#include "detail/rom_loader.hpp"

std::array<uint8_t, 0x10000> mem;

uint8_t cpu6502_read(uint16_t addr)
{
    return mem[addr];
}

void cpu6502_write(uint16_t addr, uint8_t val)
{
    mem[addr] = val;
}


void log(const char* str)
{
    fprintf(stderr, "%s", str);
}

TEST(Cpu, FunctionalInstructionTest)
{
    ASSERT_TRUE(load_rom("roms/6502_functional_test.bin", mem));

    cpu6502 cpu(cpu6502_read, cpu6502_write, log);

    cpu.state.pc = 0x400;

    cpu.run(200000*160); // enough instructions to execute the entire test ROM

    if (cpu.state.pc == 0x3469) // success loop address
    {
        SUCCEED();
    }
    else
    {
        FAIL() << "6502 test rom failure at pc = 0x" << std::hex << std::setfill('0') << std::setw(4) << cpu.state.pc << "\n" <<
                  "CPU State : \n" << std::hex << std::setfill('0') << std::setw(2) <<
                    "\tpc = 0x" << (int)cpu.state.pc << "\n" <<
                    "\ta  = 0x" << (int)cpu.state.a << ", x = 0x" << (int)cpu.state.x << ", y = 0x" << (int)cpu.state.y << "\n" <<
                    "\tflags = 0x" << (int)cpu.state.flags << "\n" <<
                    "\tsp = 0x" << (int)cpu.state.sp << " (*sp = 0x" << (int)mem[0x100 + cpu.state.sp] << ")\n" <<
                  "Check file 'roms/6502_functional_test.lst' at $pc for more information about the failure reason\n";
    }
}
