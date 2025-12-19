#pragma once

#include <Ast/astBase.hpp>
#include <Ast/functionParameters.hpp>

#include <Value/Type.hpp>
#include <fig_string.hpp>

namespace Fig::Ast
{
    class LambdaExprAst : public ExpressionAst
    {
    public:
        /*
        Lambda:
            fun (greeting) -> Null {}
        */

        FunctionParameters paras;
        FString retType;
        BlockStatement body;
        LambdaExprAst() :
            retType(ValueType::Null.name)
        {
            type = AstType::LambdaExpr;
        }
        LambdaExprAst(FunctionParameters _paras, FString _retType, BlockStatement _body) :
            retType(ValueType::Null.name)
        {
            paras = std::move(_paras);
            retType = std::move(_retType);
            body = std::move(_body);
        }

        virtual FString typeName() override
        {
            return FString(std::format("LambdaExprAst<{}>", retType.toBasicString()));
        }
        virtual ~LambdaExprAst() = default;
    };

    using LambdaExpr = std::shared_ptr<LambdaExprAst>;
}; // namespace Fig