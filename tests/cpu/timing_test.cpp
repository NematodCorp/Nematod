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

#include <iostream>
#include <cstdio>
#include <iomanip>

#include "cpu.hpp"

static std::array<uint8_t, 0x10000> mem;

static uint8_t cpu6502_read(uint16_t addr)
{
    return mem[addr];
}

static void cpu6502_write(uint16_t addr, uint8_t val)
{
    mem[addr] = val;
}


static void log(const char* str)
{
    fprintf(stderr, "%s", str);
}

#define DO_TEST(opcode, target_cycles) \
    cpu.state.pc = 0xF0; \
    cpu.cycles = 0;   \
    mem[0xF0] = (opcode);\
    cpu.run(1);       \
    EXPECT_EQ(cpu.cycles, target_cycles) << "testing opcode " << std::hex << (opcode) << "\n";

#define DO_TEST_8(opcode, data, target_cycles) \
    cpu.state.pc = 0xF0; \
    cpu.cycles = 0;   \
    mem[0xF0] = (opcode);\
    mem[0xF1] = (data);\
    cpu.run(1);       \
    EXPECT_EQ(cpu.cycles, target_cycles) << "testing opcode " << std::hex << (opcode) << "\n";

#define DO_TEST_16(opcode, data, target_cycles) \
    cpu.state.pc = 0xF0; \
    cpu.cycles = 0;   \
    mem[0xF0] = (opcode);\
    mem[0xF1] = (data)&0xFF;\
    mem[0xF2] = (data)>>8;\
    cpu.run(1);       \
    EXPECT_EQ(cpu.cycles, target_cycles) << "testing opcode " << std::hex << (opcode) << "\n";

#define BRANCH_TEST(opcode, test_true, test_false) \
    cpu.state.flags = test_false; \
    DO_TEST_8(opcode, 0x0, 2); \
    cpu.state.flags = test_true; \
    DO_TEST_8(opcode, 0x0, 3); \
    DO_TEST_8(opcode, 0x70, 4); // new page

#define ALU_TEST(base) \
    DO_TEST_8((base)+0x09, 0, 2); \
    DO_TEST_8((base)+0x05, 0, 3); \
    DO_TEST_8((base)+0x15, 0, 4); \
    DO_TEST_16((base)+0x0D, 0, 4); \
    DO_TEST_8((base)+0x1D, 0, 4); \
    DO_TEST_8((base)+0x19, 0, 4); \
    DO_TEST_8((base)+0x01, 0xFF, 6); \
    DO_TEST_8((base)+0x11, 0x00, 5); \
    DO_TEST_8((base)+0x12, 0xFE, 5); \
 \
     \
    cpu.state.x = cpu.state.y = 0xFF; \
 \
    DO_TEST_8((base)+0x1D, 0xFF, 5); \
    DO_TEST_8((base)+0x19, 0xFF, 5); \
    DO_TEST_8((base)+0x01, 0xFE, 6); \
    cpu.state.x = cpu.state.y = 0;

#define RMW_TEST(base) \
    DO_TEST((base)+0x0a, 2); \
    DO_TEST_8((base)+0x06, 0x00, 5); \
    DO_TEST_8((base)+0x16, 0x00, 6); \
    DO_TEST_16((base)+0x0E, 0x00, 6); \
    DO_TEST_8((base)+0x1E, 0x00, 7);

TEST(Cpu, TimingInstructionTest)
{
    mem[0xFE] = 0xFF; // indirect target
    mem[0xFF] = 0xFF; // indirect target

    cpu6502 cpu(cpu6502_read, cpu6502_write, log);

    DO_TEST(0xEA, 2); // nop
    DO_TEST(0xBA, 2); // tsx
    DO_TEST(0x8A, 2); // txa
    DO_TEST(0x9A, 2); // txs
    DO_TEST(0x98, 2); // tya
    DO_TEST(0xAA, 2); // tax
    DO_TEST(0xA8, 2); // tay

    DO_TEST(0x18, 2); // clc
    DO_TEST(0xD8, 2); // cld
    DO_TEST(0x58, 2); // cli
    DO_TEST(0xB8, 2); // clv

    DO_TEST(0x38, 2); // sec
    DO_TEST(0xF8, 2); // sed
    DO_TEST(0x78, 2); // sei

    DO_TEST(0x1A, 2); // inca
    DO_TEST(0xE8, 2); // inx
    DO_TEST(0xC8, 2); // iny

    DO_TEST(0x3A, 2); // deca
    DO_TEST(0xCA, 2); // dex
    DO_TEST(0x88, 2); // dey

    DO_TEST(0x00, 7); // brk
    DO_TEST(0x40, 6); // rti
    DO_TEST(0x60, 6); // rts
    DO_TEST_16(0x20, 0x0000, 6); // jsr

    DO_TEST(0x48, 3); // pha
    DO_TEST(0x08, 3); // php
    DO_TEST(0xDA, 3); // phx
    DO_TEST(0x5A, 3); // phy

    DO_TEST(0x68, 4); // pla
    DO_TEST(0x28, 4); // plp
    DO_TEST(0xFA, 4); // plx
    DO_TEST(0x7A, 4); // ply

    ALU_TEST(0x60); // adc
    ALU_TEST(0xE0); // sbc

    RMW_TEST(0x00); // asl

    BRANCH_TEST(0x90, 0b0, 0b1); // bcc
    BRANCH_TEST(0xB0, 0b1, 0b0); // bcs

    BRANCH_TEST(0xF0, 0b10, 0b0); // beq
    BRANCH_TEST(0xD0, 0b0, 0b10); // bne

    DO_TEST_8(0x89, 0, 2); // bit imm
    DO_TEST_8(0x24, 0, 3); // bit zp
    DO_TEST_8(0x34, 0, 4); // bit zpx
    DO_TEST_16(0x2C, 0, 4); // bit abs
    DO_TEST_16(0x3C, 0, 4); // bit abx

    BRANCH_TEST(0x30, 0b10000000, 0b0); // bmi
    BRANCH_TEST(0x10, 0b0, 0b10000000); // bpl

    BRANCH_TEST(0x50, 0b0, 0b1000000); // bvc
    BRANCH_TEST(0x70, 0b1000000, 0b0); // bvs

    ALU_TEST(0xC0); // cmp

    DO_TEST_8(0xE0, 0, 2); // cpx imm
    DO_TEST_8(0xE4, 0, 3); // cpx zp
    DO_TEST_8(0xEC, 0, 4); // cpx abs

    DO_TEST_8(0xC0, 0, 2); // cpy imm
    DO_TEST_8(0xC4, 0, 3); // cpy zp
    DO_TEST_8(0xCC, 0, 4); // cpy abs

    DO_TEST_8(0xC6, 0, 5); // dec zp
    DO_TEST_8(0xD6, 0, 6); // dec zpx
    DO_TEST_16(0xCE, 0, 6);// dec abs
    DO_TEST_8 (0xDE, 0, 7);// dec abx

    ALU_TEST(0x40); // eor
    ALU_TEST(0x20); // and

    DO_TEST_8(0xE6, 0, 5); // inc zp
    DO_TEST_8(0xF6, 0, 6); // inc zpx
    DO_TEST_16(0xEE, 0, 6);// inc abs
    DO_TEST_8 (0xFE, 0, 7);// inc abx

    DO_TEST_16(0x4C, 0, 3); // jmp
    DO_TEST_16(0x6C, 0, 5); // jmp ind
    DO_TEST_16(0x7C, 0, 6); // jmp absx

    ALU_TEST(0xA0); // lda

    DO_TEST_8(0xA2, 0, 2); // ldx imm
    DO_TEST_8(0xA6, 0, 3); // ldx zp
    DO_TEST_8(0xB6, 0, 4); // ldx zpy
    DO_TEST_16(0xAE, 0, 4);// ldx abs
    DO_TEST_8(0xBE, 0, 4); // ldx aby

    // page cross
    cpu.state.x = cpu.state.y = 0xFF;
    DO_TEST_8(0xBE, 0xFE, 5); // ldx aby
    cpu.state.x = cpu.state.y = 0;

    DO_TEST_8(0xA0, 0, 2); // ldy imm
    DO_TEST_8(0xA4, 0, 3); // ldy zp
    DO_TEST_8(0xB4, 0, 4); // ldy zpy
    DO_TEST_16(0xAC, 0, 4);// ldy abs
    DO_TEST_8(0xBC, 0, 4); // ldy aby

    // page cross
    cpu.state.x = cpu.state.y = 0xFF;
    DO_TEST_8(0xBC, 0xFE, 5); // ldy aby
    cpu.state.x = cpu.state.y = 0;

    RMW_TEST(0x40); // lsr

    ALU_TEST(0x00); // ora

    RMW_TEST(0x20); // rol
    RMW_TEST(0x60); // ror

    DO_TEST_8(0x85, 0, 3); // sta zp
    DO_TEST_8(0x95, 0, 4); // sta zpx
    DO_TEST_16(0x8D, 0, 4);// sta abs
    DO_TEST_8(0x9D, 0, 5); // sta abx
    DO_TEST_8(0x99, 0, 5); // sta aby
    DO_TEST_8(0x81, 0, 6); // sta indx
    DO_TEST_8(0x91, 0, 6); // sta indx
    DO_TEST_8(0x92, 0, 5); // sta izp

    DO_TEST_8(0x86, 0, 3); // stx zp
    DO_TEST_8(0x96, 0, 4); // stx zpx
    DO_TEST_16(0x8E, 0, 4);// stx abs

    DO_TEST_8(0x84, 0, 3); // sty zp
    DO_TEST_8(0x94, 0, 4); // sty zpx
    DO_TEST_16(0x8C, 0, 4);// sty abs
}
