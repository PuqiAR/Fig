#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class ReturnSt final : public StatementAst
    {
    public:
        Expression retValue;

        ReturnSt()
        {
            type = AstType::ReturnSt;
        }
        ReturnSt(Expression _retValue) :
            retValue(_retValue)
        {
            type = AstType::ReturnSt;
        }
    };

    using Return = std::shared_ptr<ReturnSt>;

    class BreakSt final : public StatementAst
    {
    public:
        BreakSt() 
        {
            type = AstType::BreakSt;
        }
    };

    using Break = std::shared_ptr<BreakSt>;

    class ContinueSt final : public StatementAst
    {
    public:
        ContinueSt()
        {
            type = AstType::ContinueSt;
        }
    };

    using Continue = std::shared_ptr<ContinueSt>;
};