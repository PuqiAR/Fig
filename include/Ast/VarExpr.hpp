#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class VarExprAst final : public ExpressionAst
    {
    public:
        const FString name;
        VarExprAst() :
            name(u8"")
        {
            type = AstType::VarExpr;
        }
        VarExprAst(FString _name) :
            name(std::move(_name))
        {
            type = AstType::VarExpr;
        }
    };

    using VarExpr = std::shared_ptr<VarExprAst>;
}; // namespace Fig