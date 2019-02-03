/*
vbi_test.cpp

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
#include "gtest/gtest.h"

#include "common/coroutine.hpp"

#include "nes.hpp"

#include "../../utils/screen_crc.hpp"

struct test_rom
{
    std::string path;
    uint32_t crc_pass;
};

const test_rom test_list[] =
{
    { "serom.nes", 0x46D0A62E },
    { "M1_P128K_C128K_W8K.nes", 0x71624440 },
    { "M1_P128K_C128K.nes", 0x13F3271A },
    { "M1_P128K_C32K_W8K.nes", 0xA2DB01 },
    { "M1_P128K_C32K.nes", 0xBA8B8C71 },
    { "M1_P128K.nes", 0x45BF568E },
};

const test_rom battery_test_list[] =
{
    { "M1_P128K_C32K_S8K.nes", 0x583A0178 },
    { "M1_P128K_C128K_S8K.nes", 0x583A0178 },
    { "M1_P512K_S8K.nes", 0x5A6216DD },
    { "M1_P512K_S32K.nes", 0xFECC56EA }

};

namespace
{

TEST(Ppu, MMC1Test)
{
    for (auto test : test_list)
    {
        NES::init();
        assert(NES::load_cartridge("roms/001/" + test.path));

        NES::power_cycle();

        for (size_t i { 0 }; i < 100; ++i)
        {
            NES::run_frame();
        }

        EXPECT_EQ(screen_crc32(), test.crc_pass) << test.path;
    }

    // run twice battery tests in order to setup save files
    for (auto test : battery_test_list)
    {
        for (size_t i { 0 }; i < 2; ++i)
        {
            if (i == 1)
                NES::load_game_battery_save_data();

            NES::init();
            assert(NES::load_cartridge("roms/001/" + test.path));

            NES::power_cycle();

            for (size_t i { 0 }; i < 100; ++i)
            {
                NES::run_frame();
            }

            if (i == 0)
                NES::save_game_battery_save_data();
        }
    }
}

}
