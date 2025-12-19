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
        FString typeName;
        Expression expr;

        VarDefAst() :
            typeName(ValueType::Any.name)
        {
            type = AstType::VarDefSt;
        }
        VarDefAst(bool _isPublic, bool _isConst, FString _name, FString _info, Expression _expr) :
            typeName(std::move(_info))
        {
            type = AstType::VarDefSt;
            isPublic = _isPublic;
            isConst = _isConst;
            name = std::move(_name);
            expr = std::move(_expr);
        }
    };

    using VarDef = std::shared_ptr<VarDefAst>;
} // namespace Fig