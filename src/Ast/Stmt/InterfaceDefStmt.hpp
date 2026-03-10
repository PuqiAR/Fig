/*!
    @file src/Ast/Stmt/InterfaceDefStmt.hpp
    @brief 接口定义 AST 节点
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct InterfaceDefStmt final : public Stmt
    {
        struct Method
        {
            String              name;
            DynArray<TypeExpr*> params;
            TypeExpr           *retType;
            SourceLocation      location;
        };

        bool            isPublic;
        String          name;
        DynArray<Method> methods;

        InterfaceDefStmt() { type = AstType::InterfaceDefStmt; }

        InterfaceDefStmt(bool _p, String _n, DynArray<Method> _m, SourceLocation _loc)
            : isPublic(_p), name(std::move(_n)), methods(std::move(_m))
        {
            type     = AstType::InterfaceDefStmt;
            location = std::move(_loc);
        }

        virtual String toString() const override { return "<InterfaceDefStmt>"; }
    };
} // namespace Fig
