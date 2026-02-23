/*!
    @file src/Ast/Expr/IndexExpr.hpp
    @brief IndexExpr定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-17
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct IndexExpr final : public Expr
    {
        Expr *base;
        Expr *index;

        IndexExpr()
        {
            type = AstType::IndexExpr;
        }

        IndexExpr(Expr *_base, Expr *_index) : base(_base), index(_index)
        {
            type = AstType::IndexExpr;
            location = base->location;
        }

        virtual String toString() const override
        {
            return std::format("<IndexExpr: '{}[{}]'>", base->toString(), index->toString());
        }
    };
}; // namespace Fig