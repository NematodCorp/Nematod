/*
standard_controller.hpp

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
#ifndef STANDARD_CONTROLLER_HPP
#define STANDARD_CONTROLLER_HPP

#include "inputadapter.hpp"

class StandardController : public InputAdapter
{
public:
    virtual void    input_write(uint8_t val);
    virtual uint8_t output1_read()          ;
    virtual uint8_t output2_read()          ;

public:
    struct State
    {
        bool a, b;
        bool select, start;
        bool up, down, left, right;
    };
    State p1_state;
    State p2_state;

private:
    bool m_strobe_on { true };
    uint8_t m_p1_button_to_output { 0 };
    uint8_t m_p2_button_to_output { 0 };

};

#endif // STANDARD_CONTROLLER_HPP
