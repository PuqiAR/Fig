#pragma once

#include <Ast/astBase.hpp>
#include <fig_string.hpp>
#include <Ast/AccessModifier.hpp>

#include <vector>

namespace Fig::Ast
{

    struct StructDefField
    {
        AccessModifier am;
        FString fieldName;
        FString tiName;
        Expression defaultValueExpr;

        StructDefField() {}
        StructDefField(AccessModifier _am, FString _fieldName, FString _tiName, Expression _defaultValueExpr) :
            am(std::move(_am)), fieldName(std::move(_fieldName)), tiName(std::move(_tiName)), defaultValueExpr(std::move(_defaultValueExpr))
        {
        }
    };
    class StructDefSt final : public StatementAst
    {
    public:
        bool isPublic;
        const FString name;
        const std::vector<StructDefField> fields; // field name (:type name = default value expression)
                                                  // name / name: String / name: String = "Fig"
        const BlockStatement body;
        StructDefSt()
        {
            type = AstType::StructSt;
        }
        StructDefSt(bool _isPublic, FString _name, std::vector<StructDefField> _fields, BlockStatement _body) :
            isPublic(std::move(_isPublic)), name(std::move(_name)), fields(std::move(_fields)), body(std::move(_body))
        {
            type = AstType::StructSt;
        }
    };

    using StructDef = std::shared_ptr<StructDefSt>;
}; // namespace Fig