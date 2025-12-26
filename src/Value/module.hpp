#pragma once

#include <Core/fig_string.hpp>

#include <Context/context_forward.hpp>

namespace Fig
{
    struct Module
    {
        FString name;
        ContextPtr ctx;

        Module() = default;

        Module(FString n, ContextPtr c) :
            name(std::move(n)),
            ctx(std::move(c))
        {
        }

        bool operator==(const Module &o) const noexcept
        {
            return name == o.name;
        }
    };
};