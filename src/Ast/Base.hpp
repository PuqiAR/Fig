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
        ObjectInitExpr, // new Point{}

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

        /* Type Expressions */
        TypeExpr,
        NamedTypeExpr,
        NullableTypeExpr
    };

    struct AstNode
    {
        AstType        type = AstType::AstNode;
        SourceLocation location;

        virtual String toString() const = 0;
        virtual ~AstNode() {};
    };

    struct TypeExpr : public AstNode
    {
        TypeExpr()
        {
            type = AstType::TypeExpr;
        }
        virtual ~TypeExpr() = default;
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
} // namespace Fig
