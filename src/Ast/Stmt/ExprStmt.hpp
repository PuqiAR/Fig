/*!
    @file src/Ast/Stmt/ExprStmt.hpp
    @brief ExprStmt定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct ExprStmt final : public Stmt
    {
        Expr *expr;

        ExprStmt()
        {
            type = AstType::ExprStmt;
        }

        ExprStmt(Expr *_expr) :
            expr(_expr)
        {
            type = AstType::ExprStmt;
            location = _expr->location;
        }

        virtual String toString() const override
        {
            return std::format("<ExprStmt: {}>", expr->toString());
        }
    };
}