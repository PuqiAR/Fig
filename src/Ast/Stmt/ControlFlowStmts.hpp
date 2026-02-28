/*!
    @file src/Ast/Stmt/ControlFlowStmts
    @brief 控制流语句 return, break, continue 定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-27
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct ReturnStmt final : public Stmt
    {
        Expr *value;

        ReturnStmt()
        {
            type = AstType::ReturnStmt;
        }

        ReturnStmt(Expr *_value, SourceLocation _location) : value(_value)
        {
            type     = AstType::ReturnStmt;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<ReturnStmt '{}'>", value->toString());
        }
    };

    struct BreakStmt final : public Stmt
    {
        BreakStmt()
        {
            type = AstType::BreakStmt;
        }

        BreakStmt(SourceLocation _location)
        {
            type = AstType::BreakStmt;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return "<BreakStmt>";
        }
    };

    struct ContinueStmt final : public Stmt
    {
        ContinueStmt()
        {
            type = AstType::ContinueStmt;
        }

        ContinueStmt(SourceLocation _location)
        {
            type     = AstType::ContinueStmt;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return "<ContinueStmt>";
        }
    };
}; // namespace Fig