/*!
    @file src/Ast/Expr/LiteralExpr.hpp
    @brief 字面量表达式定义
*/

#pragma once
#include <Ast/Base.hpp>
#include <Token/Token.hpp>

namespace Fig
{
    struct LiteralExpr final : public Expr
    {
        Token literal;

        LiteralExpr() { type = AstType::LiteralExpr; }
        LiteralExpr(const Token &_literal, SourceLocation _location) : literal(_literal)
        {
            type = AstType::LiteralExpr;
            location = std::move(_location);
        }

        virtual String toString() const override {
            return std::format("<LiteralExpr: {}>", magic_enum::enum_name(literal.type));
        }
    };
}
