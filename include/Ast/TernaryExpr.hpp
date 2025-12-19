#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    // condition ? val_true : val_false
    class TernaryExprAst final : public ExpressionAst
    {
    public:
        Expression condition;
        Expression valueT;
        Expression valueF;

        TernaryExprAst()
        {
            type = AstType::TernaryExpr;
        }
        TernaryExprAst(Expression _condition, Expression _valueT, Expression _valueF)
        {
            type = AstType::TernaryExpr;

            condition = std::move(_condition);
            valueT = std::move(_valueT);
            valueF = std::move(_valueF);
        }
    };
    using TernaryExpr = std::shared_ptr<TernaryExprAst>;
} // namespace Fig