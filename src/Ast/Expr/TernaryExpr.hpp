/*!
    @file src/Ast/Expr/TernaryExpr.hpp
    @brief cond ? then : else
    @author PuqiAR (im@puqiar.top)
    @date 2026-06-06
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct TernaryExpr final : public Expr
    {
        Expr *cond;
        Expr *thenExpr;
        Expr *elseExpr;

        TernaryExpr() { type = AstType::TernaryExpr; }

        TernaryExpr(Expr *_cond, Expr *_then, Expr *_else, SourceLocation _loc) :
            cond(_cond), thenExpr(_then), elseExpr(_else)
        {
            type     = AstType::TernaryExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            return std::format("<TernaryExpr: {} ? {} : {}>",
                cond->toString(), thenExpr->toString(), elseExpr->toString());
        }
    };
} // namespace Fig
