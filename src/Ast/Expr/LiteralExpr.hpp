/*!
    @file src/Ast/Expr/LiteralExpr.hpp
    @brief 字面量表达式定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Ast/Base.hpp>
#include <Token/Token.hpp>

namespace Fig
{
    struct LiteralExpr final : Expr
    {
        Token token;

        LiteralExpr()
        {
            type = AstType::LiteralExpr;
        }
        LiteralExpr(const Token& token) : token(token) 
        {
            type = AstType::LiteralExpr;
        }
    };
}; // namespace Fig