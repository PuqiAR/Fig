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
        bool isInfer; // 是否用了 := 类型推断
        Expr  *initExpr;

        int localId = -1;

        VarDecl()
        {
            type = AstType::VarDecl;
        }

        VarDecl(bool _isPublic, String _name, Expr *_typeSpecifier, bool _isInfer, Expr *_initExpr, SourceLocation _location) :
            name(std::move(_name)),
            typeSpecifier(_typeSpecifier),
            isInfer(_isInfer),
            initExpr(_initExpr) // location 指向关键字 var/const位置
        {
            type     = AstType::VarDecl;
            isPublic = _isPublic;
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