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