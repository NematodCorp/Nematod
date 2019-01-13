/*
coroutine.hpp

Copyright (c) 22 Yann BOUCHER (yann)

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
#ifndef COROUTINE_HPP
#define COROUTINE_HPP

#include "external/libaco/aco.h"

struct coroutine_group
{
    aco_share_stack_t* stack;
};

struct coroutine
{
    aco_t* co;
};

inline aco_t* main_coroutine { nullptr };

inline void coroutines_init()
{
    aco_thread_init(nullptr);
    main_coroutine = aco_create(nullptr, nullptr, 0, nullptr, nullptr);
}

inline coroutine_group make_co_group()
{
    return {aco_share_stack_new(4096)};
}

inline void destroy_co_group(coroutine_group& group)
{
    aco_share_stack_destroy(group.stack);
}

inline coroutine make_co(const coroutine_group& group, void(*func)(), void* arg = nullptr)
{
    return {aco_create(main_coroutine, group.stack, 64, (void(*)())func, arg)};
}

inline void destroy_co(coroutine& co)
{
    aco_destroy(co.co);
}

inline void* get_co_arg()
{
    return aco_get_arg();
}

inline void set_co_arg(coroutine& co, void* arg)
{
    co.co->arg = arg;
}

inline void run_co(const coroutine& resume_co)
{
    // don't switch to if it can be skipped
    if (resume_co.co->skip_count)
    {
        --resume_co.co->skip_count;
        return;
    }
    aco_resume(resume_co.co);
}

inline void co_set_skip(size_t count)
{
    aco_get_co()->skip_count = count;
}
inline void co_set_skip(aco_t* co, size_t count)
{
    co->skip_count = count;
}

inline void co_yield()
{
    aco_yield();
}

inline void co_exit()
{
    aco_exit();
}

#endif // COROUTINE_HPP
