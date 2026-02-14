/*!
    @file src/Ast/Expr/IdentiExpr.hpp
    @brief IdentiExpr定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once
#include <Ast/Base.hpp>

namespace Fig
{
    struct IdentiExpr final : Expr 
    {
        String name;
        
        IdentiExpr()
        {
            type = AstType::IdentiExpr;
        }

        IdentiExpr(String _name, SourceLocation _loc)
        {
            type = AstType::IdentiExpr;
            name = std::move(_name);
            location = std::move(_loc);
        }
    };
};