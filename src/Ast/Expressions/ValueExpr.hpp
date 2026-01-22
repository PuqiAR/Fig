#pragma once

#include <Ast/astBase.hpp>

#include <Evaluator/Value/value.hpp>

namespace Fig::Ast
{
    class ValueExprAst final : public ExpressionAst
    {
    public:
        ObjectPtr val;

        ValueExprAst()
        {
            type = AstType::ValueExpr;
        }
        ValueExprAst(ObjectPtr _val)
        {
            type = AstType::ValueExpr;
            val = std::move(_val);
        }
    };

    using ValueExpr = std::shared_ptr<ValueExprAst>;
};