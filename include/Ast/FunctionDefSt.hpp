#pragma once

#include <Ast/astBase.hpp>
#include <Ast/functionParameters.hpp>
#include <fig_string.hpp>

#include <value.hpp>

namespace Fig::Ast
{
    /*
        func greet(greeting, name:String, age:Int, split:String=":") -> Null
        {
            io.println("{}, {}{}{}", greeting, name, split, age);
        }

        `greeting`, `name`, `age` -> positional parameters
        `split`                   -> default parameter
        */

    class FunctionDefSt final : public StatementAst // for definition
    {
    public:
        FString name;
        FunctionParameters paras;
        bool isPublic;
        FString retType;
        BlockStatement body;
        FunctionDefSt() :
            retType(ValueType::Null.name)
        {
            type = AstType::FunctionDefSt;
        }
        FunctionDefSt(FString _name, FunctionParameters _paras, bool _isPublic, FString _retType, BlockStatement _body)
        {
            type = AstType::FunctionDefSt;

            name = std::move(_name);
            paras = std::move(_paras);
            isPublic = _isPublic;
            retType = std::move(_retType);
            body = std::move(_body);
        }
    };
    using FunctionDef = std::shared_ptr<FunctionDefSt>;
}; // namespace Fig