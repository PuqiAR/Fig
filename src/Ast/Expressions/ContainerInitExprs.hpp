
// Container Data Types --- Tuple/List/Map...

#pragma once

#include <Ast/astBase.hpp>

#include <map>

namespace Fig::Ast
{
    class ListExprAst final : public ExpressionAst
    {
    public:
        std::vector<Expression> val;

        ListExprAst()
        {
            type = AstType::ListExpr;
        }

        ListExprAst(std::vector<Expression> _val) :
            val(std::move(_val))
        {
            type = AstType::ListExpr;
        }
    };
    using ListExpr = std::shared_ptr<ListExprAst>;

    class TupleExprAst final : public ExpressionAst
    {
    public:
        std::vector<Expression> val;

        TupleExprAst()
        {
            type = AstType::TupleExpr;
        }

        TupleExprAst(std::vector<Expression> _val) :
            val(std::move(_val))
        {
            type = AstType::TupleExpr;
        }
    };
    using TupleExpr = std::shared_ptr<TupleExprAst>;

    class MapExprAst final : public ExpressionAst
    {
    public:
        std::map<Expression, Expression> val;

        MapExprAst()
        {
            type = AstType::MapExpr;
        }

        MapExprAst(std::map<Expression, Expression> _val) :
            val(std::move(_val))
        {
            type = AstType::MapExpr;
        }
    };
    using MapExpr = std::shared_ptr<MapExprAst>;
}; // namespace Fig::Ast