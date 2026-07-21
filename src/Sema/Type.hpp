/*!
    @file src/Sema/Type.hpp
    @brief 类型系统
    @author PuqiAR (im@puqiar.top)
    @date 2026-07-05
*/

#pragma once

#include <Object/ObjectBase.hpp>

namespace Fig
{
    struct Type
    {
        TypeObject *obj        = nullptr;
        bool        isNullable = false;

        bool is(TypeTag t) const;
        bool isAssignableTo(const Type &target) const;
    };

    inline bool Type::is(TypeTag t) const
    {
        return obj && obj->tag == t;
    }

    inline bool Type::isAssignableTo(const Type &target) const
    {
        if (target.is(TypeTag::Any) || is(TypeTag::Any))
            return true;
        if (is(TypeTag::Null) && target.isNullable)
            return true;
        return obj == target.obj && (!isNullable || target.isNullable);
    }
} // namespace Fig
