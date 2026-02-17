/*!
    @file src/Ast/Stmt/VarDecl.hpp
    @brief VarDecl定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-17
*/

#pragma once

#include <Ast/Base.hpp>
#include <Deps/Deps.hpp>

namespace Fig
{
    struct VarDecl final : public Stmt
    {
        String name;
        Expr  *typeSpecifier;
        Expr  *initExpr;

        VarDecl()
        {
            type = AstType::VarDecl;
        }

        VarDecl(String _name, Expr *_typeSpecifier, Expr *_initExpr, SourceLocation _location) :
            name(std::move(_name)),
            typeSpecifier(_typeSpecifier),
            initExpr(_initExpr) // location 指向关键字 var/const位置
        {
            type     = AstType::VarDecl;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            const String &typeSpecifierString = (typeSpecifier ? typeSpecifier->toString() : "Any");
            const String &initExprString = (initExpr ? initExpr->toString() : "(disprovided)");
            return std::format("<VarDecl {}: {} = {}>", name, typeSpecifierString, initExprString);
        }
    };
}; // namespace Fig