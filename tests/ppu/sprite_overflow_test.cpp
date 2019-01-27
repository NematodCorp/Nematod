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

#include "utils/blargg_tests.hpp"

namespace
{

const std::string tests[] =
{
    "01-basics.nes",
    "02-details.nes",
    "03-timing.nes",
    "04-obscure.nes",
    "05-emulator.nes",
};

TEST(Ppu, SpriteOverflowTest)
{
    std::string output;
    for (const auto& test_rom : tests)
    {
        output.clear();
        if (!do_blargg_test("roms/sprite_overflow_tests/" + test_rom, output))
        {
            ADD_FAILURE() << "Test '" << test_rom << "' failed : \n"
                   << "'" << output << "'\n"
                      ;
        }
    }
}



}
