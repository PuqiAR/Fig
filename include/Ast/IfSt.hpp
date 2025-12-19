#pragma once

#include <Ast/astBase.hpp>

namespace Fig::Ast
{
    class ElseSt final : public StatementAst
    {
    public:
        BlockStatement body;
        ElseSt()
        {
            type = AstType::ElseSt;
        }
        ElseSt(BlockStatement _body) :
            body(_body)
        {
            type = AstType::ElseSt;
        }
        virtual FString toString() override
        {
            return FString(std::format("<Else Ast at {}:{}>", aai.line, aai.column));
        }
    };
    using Else = std::shared_ptr<ElseSt>;
    class ElseIfSt final : public StatementAst
    {
    public:
        Expression condition;
        BlockStatement body;
        ElseIfSt()
        {
            type = AstType::ElseIfSt;
        }
        ElseIfSt(Expression _condition,
                 BlockStatement _body) :
            condition(_condition), body(_body)
        {
            type = AstType::ElseIfSt;
        }
        virtual FString toString() override
        {
            return FString(std::format("<ElseIf Ast at {}:{}>", aai.line, aai.column));
        }
    };
    using ElseIf = std::shared_ptr<ElseIfSt>;
    class IfSt final : public StatementAst
    {
    public:
        Expression condition;
        BlockStatement body;
        std::vector<ElseIf> elifs;
        Else els;
        IfSt()
        {
            type = AstType::IfSt;
        }
        IfSt(Expression _condition,
             BlockStatement _body,
             std::vector<ElseIf> _elifs,
             Else _els) :
            condition(_condition), body(_body), elifs(_elifs), els(_els)
        {
            type = AstType::IfSt;
        }
    };
    using If = std::shared_ptr<IfSt>;
}; // namespace Fig