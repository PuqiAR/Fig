#pragma once

#include <chrono>

namespace Fig::Time
{
    using Clock = std::chrono::steady_clock;
    extern Clock::time_point start_time; // since process start
    void init();
};