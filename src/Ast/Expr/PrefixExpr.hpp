/*!
    @file src/Ast/Expr/PrefixExpr.hpp
    @brief 前缀表达式定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Ast/Operator.hpp>
#include <Ast/Base.hpp>

#include <Deps/Deps.hpp>

namespace Fig
{
    struct PrefixExpr final : Expr
    {
        UnaryOperator op;
        Expr *operand;

        PrefixExpr()
        {
            type = AstType::PrefixExpr;
        }

        PrefixExpr(UnaryOperator _op, Expr *_operand) :
            op(_op), operand(_operand)
        {
            type = AstType::PrefixExpr;
            location = _operand->location;
        }

        virtual String toString() const override
        {
            return std::format("<PrefixExpr: {} '{}'>", magic_enum::enum_name(op), operand->toString());
        }
    };
};