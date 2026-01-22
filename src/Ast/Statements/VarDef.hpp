#pragma once

#include <Ast/astBase.hpp>
#include <Value/Type.hpp>

namespace Fig::Ast
{
    class VarDefAst final : public StatementAst
    {
    public:
        bool isPublic;
        bool isConst;
        FString name;
        // FString typeName;
        Expression declaredType;
        Expression expr;

        bool followupType;

        VarDefAst()
        {
            type = AstType::VarDefSt;
            declaredType = nullptr;
            expr = nullptr;
            followupType = false;
        }
        VarDefAst(bool _isPublic, bool _isConst, FString _name, Expression _declaredType, Expression _expr, bool _followupType)
        {
            type = AstType::VarDefSt;
            isPublic = _isPublic;
            isConst = _isConst;
            name = std::move(_name);
            declaredType = std::move(_declaredType);
            expr = std::move(_expr);
            followupType = _followupType;
        }
    };

    using VarDef = std::shared_ptr<VarDefAst>;
} // namespace Fig