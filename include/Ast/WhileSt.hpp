#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class WhileSt final : public StatementAst
    {
    public:
        Expression condition;
        BlockStatement body;

        WhileSt()
        {
            type = AstType::WhileSt;
        }

        WhileSt(Expression _condition, BlockStatement _body)
            : condition(_condition), body(_body)
        {
            type = AstType::WhileSt;
        }
    };

    using While = std::shared_ptr<WhileSt>;
};