/*!
    @file src/Ast/Stmt/ForStmt.hpp
    @brief for loop
    @author PuqiAR (im@puqiar.top)
    @date 2026-06-06
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct ForStmt final : public Stmt
    {
        Stmt      *init;
        Expr      *cond;
        Expr      *step;
        BlockStmt *body;

        ForStmt() { type = AstType::ForStmt; }

        ForStmt(Stmt *_init, Expr *_cond, Expr *_step, BlockStmt *_body, SourceLocation _loc) :
            init(_init), cond(_cond), step(_step), body(_body)
        {
            type     = AstType::ForStmt;
            location = std::move(_loc);
        }

        virtual String toString() const override { return "<ForStmt>"; }
    };
} // namespace Fig
