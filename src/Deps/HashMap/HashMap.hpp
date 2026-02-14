/*!
    @file src/Deps/HashMap/HashMap.hpp
    @brief 依赖库HashMap定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-13
*/

#pragma once

#include <unordered_map>

namespace Fig::Deps
{
    template <class _Key,
              class _Tp,
              class _Hash = std::hash<_Key>,
              class _Pred = std::equal_to<_Key>,
              class _Alloc = std::allocator<std::pair<const _Key, _Tp> >>
    using HashMap = std::unordered_map<_Key, _Tp, _Hash, _Pred, _Alloc>;
};