/*!
    @file src/Ast/Expr/ObjectInitExpr.hpp
    @brief 对象初始化表达式 AST 定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct ObjectInitExpr final : public Expr
    {
        struct Arg
        {
            String name;
            Expr  *value;
        };
        TypeExpr     *typeExpr;
        DynArray<Arg> args;

        ObjectInitExpr()
        {
            type = AstType::ObjectInitExpr;
        }
        ObjectInitExpr(TypeExpr *_te, DynArray<Arg> _args, SourceLocation _loc) :
            typeExpr(_te), args(std::move(_args))
        {
            type     = AstType::ObjectInitExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            String res = "<ObjectInitExpr 'new " + typeExpr->toString() + "{";
            for (size_t i = 0; i < args.size(); ++i)
            {
                if (!args[i].name.empty())
                    res += args[i].name + ": ";
                res += args[i].value->toString();
                if (i < args.size() - 1)
                    res += ", ";
            }
            res += "}'>";
            return res;
        }
    };
} // namespace Fig
