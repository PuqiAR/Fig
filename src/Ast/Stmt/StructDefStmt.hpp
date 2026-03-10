/*!
    @file src/Ast/Stmt/StructDefStmt.hpp
    @brief 结构体定义 AST 节点
*/

#pragma once
#include <Ast/Base.hpp>
#include <Ast/Stmt/FnDefStmt.hpp>

namespace Fig
{
    struct StructDefStmt final : public Stmt
    {
        struct Field
        {
            String    name;
            TypeExpr *type;
            bool      isPublic;
        };
        bool                  isPublic;
        String                name;
        DynArray<String>      typeParameters;
        DynArray<Field>       fields;
        DynArray<FnDefStmt *> methods;

        StructDefStmt()
        {
            type = AstType::StructDefStmt;
        }
        StructDefStmt(bool        _p,
            String                _n,
            DynArray<String>      _tp,
            DynArray<Field>       _f,
            DynArray<FnDefStmt *> _m,
            SourceLocation        _loc) :
            isPublic(_p),
            name(std::move(_n)),
            typeParameters(std::move(_tp)),
            fields(std::move(_f)),
            methods(std::move(_m))
        {
            type     = AstType::StructDefStmt;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            String detail = name;
            if (!typeParameters.empty())
            {
                detail += "<...>";
            }
            return std::format("<StructDefStmt '{}'>", detail);
        }
    };
} // namespace Fig
