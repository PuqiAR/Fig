#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class InitExprAst final : public ExpressionAst
    {
    public:
        FString structName;

        std::vector<std::pair<FString, Expression>> args;

        InitExprAst()
        {
            type = AstType::InitExpr;
        }

        InitExprAst(FString _structName, std::vector<std::pair<FString, Expression>> _args) :
            structName(std::move(_structName)), args(std::move(_args))
        {
            type = AstType::InitExpr;
        }
    };

    using InitExpr = std::shared_ptr<InitExprAst>;

}; // namespace Fig::Ast