/*!
    @file src/Deps/Deps.hpp
    @brief 依赖库集合
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-13
*/

#pragma once

#include <Core/CoreInfos.hpp>
#include <Deps/DynArray/DynArray.hpp>
#include <Deps/HashMap/HashMap.hpp>
#include <Deps/String/CharUtils.hpp>
#include <Deps/String/String.hpp>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <expected>
#include <format>

namespace Fig
{
#ifdef __FCORE_LINK_DEPS
    using Deps::String;
    using Deps::HashMap;
    using Deps::CharUtils;
    using Deps::DynArray;

    template <class _Tp, class _Err>
    using Result = std::expected<_Tp, _Err>;
#endif
}; // namespace Fig