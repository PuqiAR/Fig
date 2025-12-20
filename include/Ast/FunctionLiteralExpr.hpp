#pragma once

#include <Ast/astBase.hpp>
#include <Ast/functionParameters.hpp>
#include <fig_string.hpp>
#include <variant>

namespace Fig::Ast
{
    class FunctionLiteralExprAst final : public ExpressionAst
    {
    public:
        FunctionParameters paras;
        std::variant<BlockStatement, Expression> body;

        FunctionLiteralExprAst(FunctionParameters _paras, BlockStatement _body) :
            paras(std::move(_paras)), body(std::move(_body))
        {
            type = AstType::FunctionLiteralExpr;
        }

        FunctionLiteralExprAst(FunctionParameters _paras, Expression _exprBody) :
            paras(std::move(_paras)), body(std::move(_exprBody))
        {
            type = AstType::FunctionLiteralExpr;
        }

        bool isExprMode() const
        {
            return std::holds_alternative<Expression>(body);
        }

        BlockStatement &getBlockBody()
        {
            return std::get<BlockStatement>(body);
        }

        Expression &getExprBody()
        {
            return std::get<Expression>(body);
        }

        ~FunctionLiteralExprAst() = default;
    };

    using FunctionLiteralExpr = std::shared_ptr<FunctionLiteralExprAst>;
} // namespace Fig::Ast