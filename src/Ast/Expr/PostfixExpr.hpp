/*!
    @file src/Ast/Expr/PostfixExpr.hpp
    @brief expr++ / expr--
    @author PuqiAR (im@puqiar.top)
    @date 2026-06-06
*/

#pragma once

#include <Ast/Base.hpp>
#include <Ast/Operator.hpp>

namespace Fig
{
    struct PostfixExpr final : public Expr
    {
        UnaryOperator op;
        Expr         *operand;

        PostfixExpr() { type = AstType::PostfixExpr; }

        PostfixExpr(UnaryOperator _op, Expr *_operand) :
            op(_op), operand(_operand)
        {
            type     = AstType::PostfixExpr;
            location = _operand->location;
        }

        virtual String toString() const override
        {
            return std::format("<PostfixExpr: {} '{}'>",
                operand->toString(), magic_enum::enum_name(op));
        }
    };
} // namespace Fig
