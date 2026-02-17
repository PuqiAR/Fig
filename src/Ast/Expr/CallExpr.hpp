/*!
    @file src/Ast/Expr/CallExpr.hpp
    @brief CallExpr等定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-17
*/

#pragma once

#include <Ast/Base.hpp>
#include <Deps/Deps.hpp>

namespace Fig
{
    struct FnCallArgs
    {
        DynArray<Expr *> args;

        size_t size() const
        {
            return args.size();
        }

        String toString() const
        {
            String str = "(";
            for (const Expr *expr : args) 
            {
                if (expr != args.front())
                {
                    str += ", ";
                }
                str += expr->toString();
            }
            str += ")";
            return str;
        }
    };

    struct CallExpr final : public Expr
    {
        Expr      *callee;
        FnCallArgs args;

        CallExpr()
        {
            type = AstType::CallExpr;
        }

        CallExpr(Expr *_callee, FnCallArgs _args) : callee(_callee), args(std::move(_args))
        {
            type = AstType::CallExpr;
        }

        virtual String toString() const override
        {
            return std::format("<CallExpr: '{}{}'>", callee->toString(), args.toString());
        }
    };
} // namespace Fig