/*!
    @file src/Ast/Base.hpp
    @brief AstNode基类定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once
#include <Core/SourceLocations.hpp>
#include <Deps/Deps.hpp>

#include <cstdint>

namespace Fig
{
    enum class AstType : std::uint8_t
    {
        AstNode, // 基类
        Expr,    // 表达式
        Stmt,    // 语句

        IdentiExpr,  // 标识符表达式
        LiteralExpr, // 字面量表达式
        PrefixExpr,   // 一元 前缀表达式
        InfixExpr,  // 二元 中缀表达式
    };
    struct AstNode
    {
        AstType type = AstType::AstNode;
        SourceLocation location;

        virtual String toString() const = 0;
    };

    struct Expr : public AstNode
    {
        Expr()
        {
            type = AstType::Expr;
        }
    };

    struct Stmt : public AstNode
    {
        Stmt()
        {
            type = AstType::Stmt;
        }
    };

}; // namespace Fig