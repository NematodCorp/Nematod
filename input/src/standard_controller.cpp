/*
standard_controller.cpp

Copyright (c) 08 Yann BOUCHER (yann)

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

#include "standard_controller.hpp"

void StandardController::set_output(uint8_t byte)
{
    m_strobe_on = byte&1;
    if (m_strobe_on)
        m_button_to_output = 0;
}

uint8_t StandardController::read_data()
{
    if (m_strobe_on)
    {
        return state.a; // return state of first button if the strobe bit is on
    }
    else
    {
        if (m_button_to_output > 7)
            return 1;

        uint8_t ret;
        ret = ((uint8_t*)&state)[m_button_to_output];
        ++m_button_to_output;

        return ret&1;
    }
}
