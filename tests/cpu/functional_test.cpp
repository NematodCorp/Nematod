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
#include "nesloader.hpp"
#include "common/parallel_stepper.hpp"

namespace
{

static std::array<uint8_t, 0x10000> mem;

static uint8_t cpu6502_read(uint16_t addr)
{
    return mem[addr];
}

static void cpu6502_write(uint16_t addr, uint8_t val)
{
    mem[addr] = val;
    if (addr >= 0x8000)
    {
        printf("MMC1 reg access ? 0x%x\n", addr);
    }
}


static void log(const char* str)
{
    fprintf(stderr, "%s", str);
}

TEST(Cpu, FunctionalInstructionTest)
{
    //    ASSERT_TRUE(load_rom("roms/6502_functional_test.bin", mem));

    //    cpu6502 cpu(cpu6502_read, cpu6502_write, log);

    //    cpu.state.pc = 0x400;

    //    cpu.run(200000*160); // enough instructions to execute the entire test ROM

    //    if (cpu.state.pc == 0x3469) // success loop address
    //    {
    //        SUCCEED();
    //    }
    //    else
    //    {
    //        FAIL() << "6502 test rom failure at pc = 0x" << std::hex << std::setfill('0') << std::setw(4) << cpu.state.pc << "\n" <<
    //                  "CPU State : \n" << std::hex << std::setfill('0') << std::setw(2) <<
    //                  "\tpc = 0x" << (int)cpu.state.pc << "\n" <<
    //                  "\ta  = 0x" << (int)cpu.state.a << ", x = 0x" << (int)cpu.state.x << ", y = 0x" << (int)cpu.state.y << "\n" <<
    //                  "\tflags = 0x" << (int)cpu.state.flags << "\n" <<
    //                  "\tsp = 0x" << (int)cpu.state.sp << " (*sp = 0x" << (int)mem[0x100 + cpu.state.sp] << ")\n" <<
    //                  "Check file 'roms/6502_functional_test.lst' at $pc for more information about the failure reason\n";
    //    }
}

TEST(Cpu, NesTestRomTest)
{
//    cartdrige_data cart;
//    ASSERT_NO_THROW(cart = load_nes_file("roms/nestest.nes"));

//    memcpy(mem.data() + 0x8000, cart.prg_rom.data(), 0x4000);
//    memcpy(mem.data() + 0xC000, cart.prg_rom.data(), 0x4000);

//    cpu6502 cpu(cpu6502_read, cpu6502_write, log);

//    cpu.state.pc = 0xc000;
//    for (size_t i { 0 }; i < 100000; ++i)
//    {
//        cpu.run(1);
//        //        if (false)
//        //        {
//        //            printf("i : %d\n", i);
//        //            printf("PC : 0x%x [0x%x]\n", cpu.state.pc, mem[cpu.state.pc]);
//        //            printf("A : 0x%.2x, X : 0x%.2x, Y : 0x%.2x, P : 0x%.2x, SP : 0x%.2x\n", cpu.state.a,
//        //                                                                       cpu.state.x, cpu.state.y,
//        //                                                                       cpu.state.flags&(~0b10000), cpu.state.sp);
//        //            printf("0x%.2x%.2x : \n", mem[0x647], mem[0x11]);
//        //            printf("output : 0x%.2x%.2x\n", mem[0x2], mem[0x3]);
//        //            FAIL();
//        //        }
//    }
//        printf("PC : 0x%x [0x%x]\n", cpu.state.pc, mem[cpu.state.pc]);
//        printf("A : 0x%.2x, X : 0x%.2x, Y : 0x%.2x, P : 0x%.2x, SP : 0x%.2x\n", cpu.state.a,
//                                                                   cpu.state.x, cpu.state.y,
//                                                                   cpu.state.flags&(~0b10000), cpu.state.sp);
//        printf("0x%x%x : \n", mem[0x0647], mem[0x2FF]);
//        printf("output : 0x%.2x%.2x\n", mem[0x2], mem[0x3]);

//    EXPECT_EQ(mem[0x02], 0x00);
//    EXPECT_EQ(mem[0x03], 0x00);
}

static cpu6502 cpu(cpu6502_read, cpu6502_write, log);

TEST(Cpu, BlarggInstrTest)
{
    for (int i { 1 }; i <= 16; ++i)
    {
        char filename[64];
        snprintf(filename, 64, "roms/blargg_tests/test-%.2d.nes", i);

        cartdrige_data cart;
        ASSERT_NO_THROW(cart = load_nes_file(filename)) << filename;

        memcpy(mem.data() + 0x8000, cart.prg_rom.data(), 0x4000);
        memcpy(mem.data() + 0xC000, cart.prg_rom.data() + 0x4000, 0x4000);

        cpu.reset();

        mem[0x6000] = 0x80;

        while (mem[0x6000] == 0x80) // while tests are running
        {
            cpu.run(100);
        }

        if (mem[0x6000] == 0x00)
        {
            SUCCEED() << "Test " << i << " passed.\n";
        }
        else
        {
            printf("PC : 0x%x [0x%x]\n", cpu.state.pc, mem[cpu.state.pc]);
            printf("A : 0x%.2x, X : 0x%.2x, Y : 0x%.2x, P : 0x%.2x, SP : 0x%.2x\n", cpu.state.a,
                   cpu.state.x, cpu.state.y,
                   cpu.state.flags&(~0b10000), cpu.state.sp);
            printf("status : 0x%x \n", mem[0x6000]);
            printf("output : 0x%.2x%.2x\n", mem[0x2], mem[0x3]);
            printf("string : '%s'\n", &mem[0x6004]);
            FAIL() << "Test " << i << " failed with status code " << (int)mem[0x6000] << "\n"
                                                                                    "Output : '" << (const char*)&mem[0x6004] << "'\n";
        }
    }
}

}
