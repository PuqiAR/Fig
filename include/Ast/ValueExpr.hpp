#pragma once

#include <Ast/astBase.hpp>

#include <value.hpp>

namespace Fig::Ast
{
    class ValueExprAst final : public ExpressionAst
    {
    public:
        Object val;

        ValueExprAst()
        {
            type = AstType::ValueExpr;
        }
        ValueExprAst(Object _val)
        {
            type = AstType::ValueExpr;
            val = std::move(_val);
        }
    };

    using ValueExpr = std::shared_ptr<ValueExprAst>;
};