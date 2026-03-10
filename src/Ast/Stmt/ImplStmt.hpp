/*!
    @file src/Ast/Stmt/ImplStmt.hpp
    @brief 实现块 AST 节点
*/

#pragma once
#include <Ast/Base.hpp>
#include <Ast/Stmt/FnDefStmt.hpp>

namespace Fig
{
    struct ImplStmt final : public Stmt
    {
        TypeExpr             *interfaceType;
        TypeExpr             *structType;
        DynArray<FnDefStmt *> methods;

        ImplStmt(TypeExpr *_it, TypeExpr *_st, DynArray<FnDefStmt *> _m, SourceLocation _loc) :
            interfaceType(_it), structType(_st), methods(std::move(_m))
        {
            type     = AstType::ImplStmt;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            String detail =
                (interfaceType ? interfaceType->toString() + " for " : "") + structType->toString();
            return std::format("<ImplStmt '{}'>", detail);
        }
    };
} // namespace Fig
