/*!
    @file src/Core/RuntimeTime.hpp
    @brief 系统时间库定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <chrono>

namespace Fig::Time
{
    using Clock = std::chrono::steady_clock;
    extern Clock::time_point start_time; // since process start
    void init();
};