#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class VarAssignSt final : public StatementAst
    {
    public:
        const FString varName;
        const Expression valueExpr;

        VarAssignSt()
        {
            type = AstType::VarAssignSt;
        }

        VarAssignSt(FString _varName, Expression _valueExpr) :
            varName(std::move(_varName)), valueExpr(std::move(_valueExpr))
        {
            type = AstType::VarAssignSt;
        }
    };

    using VarAssign = std::shared_ptr<VarAssignSt>;
}; // namespace Fig