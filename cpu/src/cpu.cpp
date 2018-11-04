/*
cpu65c02.cpp

Copyright (c) 27 Yann BOUCHER (yann)

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

#include "cpu.hpp"

#include <array>
#include <cstdio>

extern std::array<void(*)(cpu65c02&), 256> opcodes;

cpu65c02::cpu65c02(cpu65c02::ReadCallback read_clbk, cpu65c02::WriteCallback write_clbk, LogCallback log_clbk)
    : read(read_clbk), write(write_clbk), log(log_clbk)
{
    reset();
}

void cpu65c02::reset()
{
    state.a = state.x = state.y = 0;
    state.flags = 0b00110100;
    state.sp = 0xFD;
    state.pc = 0xFFFC;
}

void cpu65c02::run(unsigned steps)
{
    for (size_t i { 0 }; i < steps; ++i)
    {
        uint8_t opcode = read(state.pc++);

        auto operation = opcodes[opcode];
        operation(*this);

        m_cycles += 2;
    }
}

void cpu65c02::branch_on(uint16_t addr, bool cond)
{
    if (cond)
    {
        if ((addr&0xFF00) != (state.pc&0xFF00)) // diff page
        {
            m_cycles += 1;
        }
        m_cycles += 1;

        state.pc = addr;
    }
}

