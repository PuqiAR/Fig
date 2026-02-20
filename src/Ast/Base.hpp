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
        AstNode,   // 基类
        Program,   // 程序
        Expr,      // 表达式
        Stmt,      // 语句
        BlockStmt, // 块语句

        /* Expressions */
        IdentiExpr,  // 标识符表达式
        LiteralExpr, // 字面量表达式
        PrefixExpr,  // 一元 前缀表达式
        InfixExpr,   // 二元 中缀表达式

        IndexExpr, // 后缀表达式，索引
        CallExpr,  // 后缀表达式，函数调用

        /* Statements */
        ExprStmt,   // 表达式语句，如 println(1)
        VarDecl,    // 变量声明
        IfStmt,     // If语句
        ElseIfStmt, // ElseIf语句，不准悬空
    };
    struct AstNode
    {
        AstType        type = AstType::AstNode;
        SourceLocation location;

        virtual String toString() const = 0;
        virtual ~AstNode(){};
    };

    struct Program;

    struct Expr : public AstNode
    {
        Expr()
        {
            type = AstType::Expr;
        }
    };

    struct Stmt : public AstNode
    {
        bool isPublic;
        Stmt()
        {
            type = AstType::Stmt;
        }
    };

    struct Program final : public AstNode
    {
        DynArray<Stmt *> nodes;

        Program()
        {
            type = AstType::Program;
        }

        Program(DynArray<Stmt *> _nodes)
        {
            type  = AstType::Program;
            nodes = std::move(_nodes);
            if (!_nodes.empty())
            {
                location = std::move(_nodes.back()->location);
            }
        }

        virtual String toString() const override
        {
            return "<Program>";
        }
    };

    struct BlockStmt final : public Stmt
    {
        DynArray<Stmt *> nodes;
        BlockStmt()
        {
            type = AstType::BlockStmt;
        }
        BlockStmt(DynArray<Stmt *> _nodes)
        {
            type  = AstType::BlockStmt;
            nodes = std::move(_nodes);
            if (!_nodes.empty())
            {
                location = std::move(_nodes.back()->location);
            }
        }
        virtual String toString() const override
        {
            return "<BlockStmt>";
        }
    };
}; // namespace Fig

namespace std
{
    template <>
    struct std::formatter<Fig::AstNode *, char>
    {
        constexpr auto parse(std::format_parse_context &ctx)
        {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const Fig::AstNode *_node, FormatContext &ctx) const
        {
            return std::format_to(ctx.out(), "{}", _node->toString().toStdString());
        }
    };
}; // namespace std