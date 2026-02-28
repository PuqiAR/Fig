/*!
    @file src/Ast/Stmt/FnDefStmt.hpp
    @brief FnDefStmt定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-25
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct Param
    {
        String         name;
        SourceLocation location;

        TypeInfo *resolvedType = nullptr;
        int       localId      = -1;

        virtual String toString() const = 0;
    };

    struct PosParam final : public Param
    {
        TypeExpr *type;
        Expr     *defaultValue;

        PosParam() {}
        PosParam(String _name, TypeExpr *_type, Expr *_defaultValue, SourceLocation _location) :
            type(_type), defaultValue(_defaultValue)
        {
            name     = std::move(_name);
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            return std::format("<Pos {}: {}{}>",
                name,
                (type ? type->toString() : "Any"),
                (defaultValue ? " =" + defaultValue->toString() : ""));
        }
    };

    /*
        (public) func foo([name: (type) (= default value)]) (-> return type)
        {
            ...
        }

    */

    struct FnDefStmt final : public Stmt
    {
        String            name;
        DynArray<Param *> params;
        TypeExpr         *returnType;
        BlockStmt        *body;

        TypeInfo *resolvedReturnType = nullptr;
        int       localId            = -1;

        FnDefStmt()
        {
            type = AstType::FnDefStmt;
        }

        FnDefStmt(bool        _isPublic,
            String            _name,
            DynArray<Param *> _params,
            TypeExpr         *_returnType,
            BlockStmt        *_body,
            SourceLocation    _location) :
            name(std::move(_name)), params(std::move(_params)), returnType(_returnType), body(_body)
        {
            type     = AstType::FnDefStmt;
            isPublic = _isPublic;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            String pStr;
            for (const Param *p : params)
            {
                if (p != *params.begin())
                {
                    pStr += ", ";
                }
                pStr += p->toString();
            }
            return std::format("<FnDefStmt {}{}({}) -> {} {{{}}}>",
                (isPublic ? "public " : ""),
                name,
                pStr,
                (returnType ? returnType->toString() : "Any"),
                body->toString());
        }
    };
}; // namespace Fig