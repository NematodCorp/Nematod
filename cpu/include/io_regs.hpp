/*
io_regs.hpp

Copyright (c) 23 Yann BOUCHER (yann)

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
#ifndef IO_REGS_HPP
#define IO_REGS_HPP

#include "memory/include/memory.hpp"

#include "common/coroutine.hpp"

class PPU;
class cpu6502;
class InputAdapter;

class IORegs : public MemoryInterfaceable
{
public:
    enum regs : uint8_t
    {
        OAMDMA = 0x14
    };

    IORegs(cpu6502& cpu, aco_t* cpu_co, PPU& ppu, InputAdapter& input) : MemoryInterfaceable(0x20),
        m_cpu(cpu), m_cpu_co(cpu_co), m_ppu(ppu), m_input(input)
    {}

protected:
    data  read(address ptr)             override;
    void write(address ptr, data value) override;

private:
    void oam_dma(uint8_t page);

    void input_write(uint8_t val);
    uint8_t input_read16();
    uint8_t input_read17();

    uint8_t invalid_read();
    void    invalid_write(uint8_t val);

public:
    using read_callback  = uint8_t(IORegs::*)();
    using write_callback = void(IORegs::*)(uint8_t);

    static std::array<read_callback, 0x20> m_read_clbks;
    static std::array<write_callback, 0x20> m_write_clbks;

    cpu6502& m_cpu;
    aco_t* m_cpu_co;
    PPU& m_ppu;
    InputAdapter& m_input;
};

#endif // IO_REGS_HPP
