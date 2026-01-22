#pragma once

#include "Value/value.hpp"
#include <Core/fig_string.hpp>
#include <Value/value_forward.hpp>
#include <Value/Type.hpp>
#include <array>
#include <memory>

namespace Fig
{
    class IntPool
    {
    private:
        static constexpr ValueType::IntClass CACHE_MIN = -128;
        static constexpr ValueType::IntClass CACHE_MAX = 127;

        std::array<ObjectPtr, CACHE_MAX - CACHE_MIN + 1> cache;

    public:
        IntPool()
        {
            for (ValueType::IntClass i = CACHE_MIN; i <= CACHE_MAX; ++i)
            {
                cache[i - CACHE_MIN] = std::make_shared<Object>(i);
            }
        }
        ObjectPtr createInt(ValueType::IntClass val) const
        {
            if (val >= CACHE_MIN && val <= CACHE_MAX) { return cache[val - CACHE_MIN]; }
            return std::make_shared<Object>(val);
        }

        static const IntPool &getInstance()
        {
            static IntPool pool;
            return pool;
        }
    };
}; // namespace Fig