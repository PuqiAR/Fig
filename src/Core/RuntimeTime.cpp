/*!
    @file src/Core/RuntimeTime.cpp
    @brief 系统时间库实现(steady_clock)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <Core/RuntimeTime.hpp>

#include <cassert>

namespace Fig::Time
{
    Clock::time_point start_time;
    void init()
    {
        static bool flag = false;
        if (flag)
        {
            assert(false);
        }
        start_time = Clock::now();
        flag = true;
    }
};