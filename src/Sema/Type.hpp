/*!
    @file src/Sema/Type.hpp
    @brief 前端类型检查的类型定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-23
*/

#pragma once

#include <cstdint>

namespace Fig
{
    enum class TypeTag : std::uint8_t
    {
        Any,  // 动态类型底线
        Null, // 空值
        Int,
        Double,
        Bool,
        String,
        Function,
        Struct,
    };

    // TODO: 复杂类型的推导(泛型，结构体)
    // 添加 TypeInfo 结构体，目前先用 TypeTag
};