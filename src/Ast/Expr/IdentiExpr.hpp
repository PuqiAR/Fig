/*!
    @file src/Ast/Expr/IdentiExpr.hpp
    @brief 标识符表达式定义
*/

#pragma once
#include <Ast/Base.hpp>
#include <Sema/Environment.hpp>

namespace Fig
{
    struct IdentiExpr final : public Expr
    {
        String  name;
        Symbol *resolvedSymbol = nullptr; // 语义分析后填充，Compiler 唯一的依赖

        IdentiExpr()
        {
            type = AstType::IdentiExpr;
        }
        IdentiExpr(String _name, SourceLocation _location) : name(std::move(_name))
        {
            type     = AstType::IdentiExpr;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<IdentiExpr '{}'>", name);
        }
    };
} // namespace Fig
