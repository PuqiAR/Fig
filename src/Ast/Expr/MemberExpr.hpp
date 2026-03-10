/*!
    @file src/Ast/Expr/MemberExpr.hpp
    @brief 成员访问表达式定义：obj.member
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct MemberExpr final : public Expr
    {
        Expr  *target; // 访问对象
        String name;   // 成员名字

        MemberExpr()
        {
            type = AstType::MemberExpr;
        }

        MemberExpr(Expr *_t, String _n, SourceLocation _loc) : target(_t), name(std::move(_n))
        {
            type     = AstType::MemberExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            return std::format("<MemberExpr {}.{}>", target->toString(), name);
        }
    };
} // namespace Fig
