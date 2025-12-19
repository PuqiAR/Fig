#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class ExpressionStmtAst final : public StatementAst
    {
    public:
        Expression exp;
        ExpressionStmtAst()
        {
            type = AstType::ExpressionStmt;
        }
        ExpressionStmtAst(Expression _exp) : exp(std::move(_exp))
        {
            type = AstType::ExpressionStmt;
        }
    };
    using ExpressionStmt = std::shared_ptr<ExpressionStmtAst>;
}
