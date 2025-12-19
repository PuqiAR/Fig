#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class BinaryExprAst final : public ExpressionAst
    {
    public:
        Operator op;
        Expression lexp, rexp;

        BinaryExprAst()
        {
            type = AstType::BinaryExpr;
        }
        BinaryExprAst(Expression _lexp, Operator _op, Expression _rexp)
        {
            type = AstType::BinaryExpr;

            lexp = _lexp;
            op = _op;
            rexp = _rexp;
        }
    };

    using BinaryExpr = std::shared_ptr<BinaryExprAst>;

}; // namespace Fig