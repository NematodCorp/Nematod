/*
inputadapter.cpp

Copyright (c) 20 Yann BOUCHER (yann)

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

#include "inputadapter.hpp"


void InputAdapter::input_write(uint8_t val)
{
    if (controller_1)
        controller_1->set_output(val & 0b111); // only first 3 bits
    if (controller_2)
        controller_2->set_output(val & 0b111); // only first 3 bits
}

uint8_t InputAdapter::output1_read()
{
    if (controller_1)
        return 0x40 | controller_1->read_data();
    else
        return 0x40; // open bus
}

uint8_t InputAdapter::output2_read()
{
    if (controller_2)
        return 0x40 | controller_2->read_data();
    else
        return 0x40; // open bus
}
