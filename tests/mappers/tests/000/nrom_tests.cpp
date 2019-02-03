/*
cnrom_tests.cpp

Copyright (c) 01 Yann BOUCHER (yann)

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

#include "common/coroutine.hpp"

#include "nes.hpp"

#include "../../utils/screen_crc.hpp"

#include "common/log.hpp"

struct coroutines_init_wrapper
{
    coroutines_init_wrapper()
    {
        coroutines_init();
        global_logger.filter(WARNING);
    }
};
static coroutines_init_wrapper co_init_instance;

struct test_rom
{
    std::string path;
    uint32_t crc_pass;
};

const test_rom test_list[] =
{
    { "M0_P32K_C8K_V.nes", 0x38DFE93D },
};

namespace
{

TEST(Ppu, NROMTest)
{
    for (auto test : test_list)
    {
        NES::init();
        assert(NES::load_cartridge("roms/000/" + test.path));

        NES::power_cycle();

        // run for ten frames
        for (size_t i { 0 }; i < 5; ++i)
        {
            NES::run_frame();
        }

        EXPECT_EQ(screen_crc32(), test.crc_pass);
    }
}



}
