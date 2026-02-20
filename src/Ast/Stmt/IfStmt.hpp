/*!
    @file src/Ast/Stmt/IfStmt.hpp
    @brief IfStmt定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-20
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct ElseIfStmt final : public Stmt
    {
        Expr      *cond;
        BlockStmt *consequent;

        ElseIfStmt()
        {
            type = AstType::ElseIfStmt;
        }

        ElseIfStmt(Expr *_cond, BlockStmt *_consequent, SourceLocation _location) :
            cond(_cond), consequent(_consequent)
        {
            type     = AstType::ElseIfStmt;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<ElseIf ({}) then {}>", cond->toString(), consequent->toString());
        }
    };

    struct IfStmt final : public Stmt
    {
        Expr                  *cond;
        BlockStmt             *consequent;
        DynArray<ElseIfStmt *> elifs;
        BlockStmt             *alternate;

        IfStmt()
        {
            type = AstType::IfStmt;
        }

        IfStmt(Expr               *_cond,
            BlockStmt             *_consequent,
            DynArray<ElseIfStmt *> _elifs,
            BlockStmt             *_alternate,
            SourceLocation         _location) :
            cond(_cond), consequent(_consequent), elifs(std::move(_elifs)), alternate(_alternate)
        {
            type     = AstType::IfStmt;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<If ({}) then {}, else if * {}, else {}>",
                cond->toString(),
                consequent->toString(),
                elifs.size(),
                alternate->toString());
        }
    };
}; // namespace Fig