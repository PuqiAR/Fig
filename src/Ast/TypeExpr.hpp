/*!
    @file src/Ast/TypeExpr.hpp
    @brief 类型表达式 AST 定义：支持泛型与空安全
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct NamedTypeExpr final : public TypeExpr
    {
        DynArray<String>     path;
        DynArray<TypeExpr *> arguments;

        NamedTypeExpr()
        {
            type = AstType::NamedTypeExpr;
        }
        NamedTypeExpr(DynArray<String> _p, DynArray<TypeExpr *> _args, SourceLocation _loc) :
            path(std::move(_p)), arguments(std::move(_args))
        {
            type     = AstType::NamedTypeExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            String detail = "";
            for (size_t i = 0; i < path.size(); ++i)
            {
                detail += path[i];
                if (i < path.size() - 1)
                    detail += ".";
            }
            if (!arguments.empty())
            {
                detail += "<";
                for (size_t i = 0; i < arguments.size(); ++i)
                {
                    detail += arguments[i]->toString();
                    if (i < arguments.size() - 1)
                        detail += ", ";
                }
                detail += ">";
            }
            return std::format("<NamedTypeExpr '{}'>", detail);
        }
    };

    struct NullableTypeExpr final : public TypeExpr
    {
        TypeExpr *inner;

        NullableTypeExpr(TypeExpr *_inner, SourceLocation _loc) : inner(_inner)
        {
            type     = AstType::NullableTypeExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            return std::format("<NullableTypeExpr '{}?'>", inner->toString());
        }
    };
} // namespace Fig
