#pragma once

#include <Core/CoreInfos.hpp>
#include <Deps/HashMap/HashMap.hpp>
#include <Deps/String/String.hpp>
#include <Deps/String/CharUtils.hpp>

#include <expected>

namespace Fig
{
    #ifdef __FCORE_LINK_DEPS
        using Deps::String;
        using Deps::HashMap;
        using Deps::CharUtils;

        template<class _Tp, class _Err>
        using Result = std::expected<_Tp, _Err>;
    #endif
};