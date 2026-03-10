/*!
    @file src/Ast/Stmt/FnDefStmt.hpp
    @brief 函数定义 AST 节点
*/

#pragma once
#include <Ast/Base.hpp>
#include <Sema/Environment.hpp>

namespace Fig
{
    struct Param : public AstNode {
        String    name;
        TypeExpr *typeSpecifier;
        Expr     *defaultValue;
        Type      resolvedType;
        Param() { type = AstType::AstNode; }
        virtual ~Param() = default;
    };

    struct PosParam final : public Param {
        PosParam(String _n, TypeExpr *_ts, Expr *_dv, SourceLocation _loc) {
            name = std::move(_n); typeSpecifier = _ts; defaultValue = _dv; location = std::move(_loc);
        }
        virtual String toString() const override { return name; }
    };

    struct FnDefStmt final : public Stmt {
        String           name;
        DynArray<Param *> params;
        TypeExpr        *returnTypeSpecifier;
        BlockStmt       *body;
        Type             resolvedReturnType;
        Symbol          *resolvedSymbol = nullptr; // 连接物理符号

        FnDefStmt() { type = AstType::FnDefStmt; }
        FnDefStmt(bool _p, String _n, DynArray<Param *> _pa, TypeExpr *_rt, BlockStmt *_b, SourceLocation _loc)
            : name(std::move(_n)), params(std::move(_pa)), returnTypeSpecifier(_rt), body(_b)
        {
            type = AstType::FnDefStmt; isPublic = _p; location = std::move(_loc);
        }

        virtual String toString() const override {
            return std::format("<FnDefStmt '{}'>", name);
        }
    };
}
