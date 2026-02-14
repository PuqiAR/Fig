/*!
    @file src/Ast/Expr/LiteralExpr.hpp
    @brief 字面量表达式定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Ast/Base.hpp>
#include <Token/Token.hpp>

#include <Deps/Deps.hpp>

namespace Fig
{
    struct LiteralExpr final : Expr
    {
        Token token;

        LiteralExpr()
        {
            type = AstType::LiteralExpr;
        }
        LiteralExpr(const Token& token, SourceLocation _location) : token(token) 
        {
            type = AstType::LiteralExpr;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<LiteralExpr: {}>", magic_enum::enum_name(token.type));
        }
    };
}; // namespace Fig