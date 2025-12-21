#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class ForSt final : public StatementAst
    {
    public:
        Statement initSt;
        Expression condition;
        Statement incrementSt;
        BlockStatement body;

        ForSt()
        {
            type = AstType::ForSt;
        }
        ForSt(Statement _initSt, Expression _condition, Statement _incrementSt, BlockStatement _body) :
            initSt(std::move(_initSt)),
            condition(std::move(_condition)),
            incrementSt(std::move(_incrementSt)),
            body(std::move(_body))
        {
            type = AstType::ForSt;
        }
    };

    using For = std::shared_ptr<ForSt>; 
};