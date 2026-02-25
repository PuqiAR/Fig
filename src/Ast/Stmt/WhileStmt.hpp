/*!
    @file src/Ast/Stmt/WhileStmt.hpp
    @brief WhileStmt定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-24
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct WhileStmt final : public Stmt
    {
        Expr *cond;
        BlockStmt *body;

        WhileStmt()
        {
            type = AstType::WhileStmt;
        }

        WhileStmt(Expr *_cond, BlockStmt *_body, SourceLocation _location) :
            cond(_cond),
            body(_body)
        {
            type = AstType::WhileStmt;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<WhileStmt ({}) {{{}}}>", cond->toString(), body->toString());
        }
    };
}; // namespace Fig