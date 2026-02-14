/*!
    @file src/Ast/Expr/InfixExpr.hpp
    @brief 中缀表达式定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Ast/Base.hpp>
#include <Ast/Operator.hpp>

#include <Deps/Deps.hpp>

namespace Fig
{
    struct InfixExpr final : Expr
    {
        Expr          *left;
        BinaryOperator op;
        Expr          *right;

        InfixExpr()
        {
            type = AstType::InfixExpr;
        }
        InfixExpr(Expr *_left, BinaryOperator _op, Expr *_right) :
            left(_left), op(_op), right(_right)
        {
            type = AstType::InfixExpr;
            location = _left->location;
        }

        virtual String toString() const override
        {
            return std::format("<InfixExpr: '{}' {} '{}'>", left->toString(), magic_enum::enum_name(op), right->toString());
        }
    };
}; // namespace Fig