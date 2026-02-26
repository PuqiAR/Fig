/*!
    @file src/Ast/TypeExpr.hpp
    @brief TypeExpr定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-25
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{

    struct NamedTypeExpr final : public TypeExpr
    {
        DynArray<String> path; // {"std", "file"} etc.

        NamedTypeExpr()
        {
            type = AstType::NamedTypeExpr;
        }

        NamedTypeExpr(DynArray<String> _path, SourceLocation _location) :
            path(std::move(_path))
        {
            type = AstType::NamedTypeExpr;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<NamedTypeExpr '{}'>", path);
        }
    };
};