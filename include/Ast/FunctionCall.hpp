// include/Ast/FunctionCall.hpp
#pragma once

#include <Ast/astBase.hpp>
#include <value.hpp>

namespace Fig::Ast
{
    struct FunctionArguments
    {
        std::vector<Expression> argv;
        size_t getLength() const { return argv.size(); }
    };

    struct FunctionCallArgs final 
    {
        std::vector<Value> argv;
        size_t getLength() const { return argv.size(); }
    };

    class FunctionCallExpr final : public ExpressionAst
    {
    public:
        FString name;
        FunctionArguments arg;

        FunctionCallExpr()
        {
            type = AstType::FunctionCall;
        }

        FunctionCallExpr(FString _name, FunctionArguments _arg) :
            name(std::move(_name)), arg(std::move(_arg))
        {
            type = AstType::FunctionCall;
        }

        virtual FString toString() override 
        {
            FString s = name;
            s += u8"(";
            for (size_t i = 0; i < arg.argv.size(); ++i)
            {
                s += arg.argv[i]->toString();
                if (i + 1 < arg.argv.size())
                    s += u8", ";
            }
            s += u8")";
            return s;
        }
    };

    using FunctionCall = std::shared_ptr<FunctionCallExpr>;
}; // namespace Fig
