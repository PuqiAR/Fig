/*!
    @file src/Ast/Base.hpp
    @brief AstNode基类定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once
#include <Core/SourceLocations.hpp>
#include <Deps/Deps.hpp>
#include <Sema/Type.hpp>
#include <cstdint>

namespace Fig
{
    enum class AstType : std::uint8_t
    {
        AstNode,
        Program,
        Expr,
        Stmt,
        BlockStmt,

        /* Expressions */
        IdentiExpr,
        LiteralExpr,
        PrefixExpr,
        InfixExpr,
        IndexExpr,
        CallExpr,
        MemberExpr,     // obj.prop
        NewExpr, // new Point{}
        LambdaExpr,
        TernaryExpr,    // cond ? then : else
        PostfixExpr,     // expr++ / expr--

        /* Statements */
        ExprStmt,
        VarDecl,
        IfStmt,
        ElseIfStmt,
        WhileStmt,
        FnDefStmt,
        StructDefStmt,
        InterfaceDefStmt,
        ImplStmt, // impl Document for File {}
        ReturnStmt,
        BreakStmt,
        ContinueStmt,
        ForStmt,        // for loop
        ImportStmt,     // import

        /* Type Expressions */
        TypeExpr,
        NamedTypeExpr,   // 废弃，用 IdentiExpr/MemberExpr/ApplyExpr 替代
        NullableTypeExpr, // 废弃，用 NullableExpr 替代
        FnTypeExpr,
        ApplyExpr,      // 泛型实例化: List<Int>
        NullableExpr,   // 可空后缀: Int?
    };

    struct AstNode
    {
        AstType        type = AstType::AstNode;
        SourceLocation location;

        virtual String toString() const = 0;
        virtual ~AstNode() {};
    };

    struct Expr : public AstNode
    {
        // 语义分析后填充
        Type resolvedType;

        Expr()
        {
            type = AstType::Expr;
        }
    };

    struct Stmt : public AstNode
    {
        bool isPublic = false;
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
        virtual String toString() const override
        {
            return "<BlockStmt>";
        }
    };

    // --- Type Expressions (inherit Expr — 类型即值) ---

    struct TypeExpr : public Expr
    {
        TypeExpr() { type = AstType::TypeExpr; }
        virtual ~TypeExpr() = default;
    };

    // ApplyExpr: 泛型实例化，List<Int> → ApplyExpr(base, [Int])
    struct ApplyExpr final : public Expr
    {
        Expr           *base; // 基础类型表达式
        DynArray<Expr *> args; // 泛型参数

        ApplyExpr() { type = AstType::ApplyExpr; }
        ApplyExpr(Expr *_base, DynArray<Expr *> _args, SourceLocation _loc) :
            base(_base), args(std::move(_args))
        {
            type     = AstType::ApplyExpr;
            location = std::move(_loc);
        }
        virtual String toString() const override
        {
            String s = base->toString() + "<";
            for (size_t i = 0; i < args.size(); ++i)
            {
                if (i) s += ", ";
                s += args[i]->toString();
            }
            s += ">";
            return s;
        }
    };

    // NullableExpr: 可空后缀 Int? → NullableExpr(Int)
    struct NullableExpr final : public Expr
    {
        Expr *inner;

        NullableExpr() { type = AstType::NullableExpr; }
        NullableExpr(Expr *_inner, SourceLocation _loc) : inner(_inner)
        {
            type     = AstType::NullableExpr;
            location = std::move(_loc);
        }
        virtual String toString() const override
        {
            return inner->toString() + "?";
        }
    };
} // namespace Fig
