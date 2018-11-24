/*
parallel_stepper.cpp

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

#include "gtest/gtest.h"

#include "common/parallel_stepper.hpp"
#include "clock.hpp"

struct coroutines_init_wrapper
{
    coroutines_init_wrapper()
    {
        coroutines_init();
    }
};
static coroutines_init_wrapper co_init_instance;

void inc_foo()
{
    while (true)
    {
        int* val = (int*)get_co_arg();
        ++(*val);
        co_yield();
    }
}
void inc_bar()
{
    while (true)
    {
        int* val = (int*)get_co_arg();
        ++(*val);
        co_yield();
    }
}

TEST(ParallelStepper, Basic)
{
    ParallelStepper<inc_foo, inc_bar> stepper;

    int foo_val = 0;
    int bar_val = 0;
    stepper.set_arg(0, &foo_val);
    stepper.set_arg(1, &bar_val);

    for (size_t i { 0 }; i < 3; ++i)
        stepper.step();

    EXPECT_EQ(foo_val, 3);
    EXPECT_EQ(bar_val, 3);

    foo_val = 0;

    for (size_t i { 0 }; i < 3; ++i)
        stepper.step();

    EXPECT_EQ(foo_val, 3);
    EXPECT_EQ(bar_val, 6);
}

template<unsigned int R>
struct ClockCounter : public DividedClockReceiver<R> {
    void on_active_clock() override {counter++;};
    unsigned int counter = 0;
};

void step_clock_foo()
{
    while (true)
    {
        auto clock = (ClockCounter<1>*)get_co_arg();
        clock->tick();

        co_yield();
    }
}
void step_clock_bar()
{
    while (true)
    {
        auto clock = (ClockCounter<3>*)get_co_arg();
        clock->tick();

        co_yield();
    }
}

TEST(ParallelStepper, Clocks)
{
    ClockCounter<1> foo;
    ClockCounter<3> bar;

    ParallelStepper<step_clock_foo, step_clock_bar> stepper;

    stepper.set_arg(0, &foo);
    stepper.set_arg(1, &bar);

    for (size_t i { 0 }; i < 300; ++i)
        stepper.step();

    EXPECT_EQ(foo.counter, 300);
    EXPECT_EQ(bar.counter, 100);
}
