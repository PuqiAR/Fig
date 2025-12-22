#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class InitExprAst final : public ExpressionAst
    {
    public:
        FString structName;

        std::vector<std::pair<FString, Expression>> args;

        enum class InitMode
        {
            Positional = 1,
            Named,
            Shorthand
        } initMode;

        /*
            3 ways of calling constructor
            .1 Person {"Fig", 1, "IDK"};
            .2 Person {name: "Fig", age: 1, sex: "IDK"}; // can be unordered
            .3 Person {name, age, sex};
        */

        InitExprAst()
        {
            type = AstType::InitExpr;
        }

        InitExprAst(FString _structName, std::vector<std::pair<FString, Expression>> _args, InitMode _initMode) :
            structName(std::move(_structName)), args(std::move(_args)), initMode(_initMode)
        {
            type = AstType::InitExpr;
        }
    };

    using InitExpr = std::shared_ptr<InitExprAst>;

}; // namespace Fig::Ast