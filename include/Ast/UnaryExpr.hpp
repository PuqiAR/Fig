#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class UnaryExprAst final : public ExpressionAst
    {
    public:
        Operator op;
        Expression exp;

        UnaryExprAst()
        {
            type = AstType::UnaryExpr;
        }
        UnaryExprAst(Operator _op, Expression _exp)
        {
            type = AstType::UnaryExpr;

            op = _op;
            exp = std::move(_exp);
        }
    };

    using UnaryExpr = std::shared_ptr<UnaryExprAst>;
} // namespace Fig