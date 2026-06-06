/*!
    @file src/Ast/TypeExpr.hpp
    @brief 类型表达式 AST 定义：支持泛型与空安全
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct NamedTypeExpr final : public TypeExpr
    {
        DynArray<String>     path;
        DynArray<Expr *> arguments;

        NamedTypeExpr()
        {
            type = AstType::NamedTypeExpr;
        }
        NamedTypeExpr(DynArray<String> _p, DynArray<Expr *> _args, SourceLocation _loc) :
            path(std::move(_p)), arguments(std::move(_args))
        {
            type     = AstType::NamedTypeExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            String detail = "";
            for (size_t i = 0; i < path.size(); ++i)
            {
                detail += path[i];
                if (i < path.size() - 1)
                    detail += ".";
            }
            if (!arguments.empty())
            {
                detail += "<";
                for (size_t i = 0; i < arguments.size(); ++i)
                {
                    detail += arguments[i]->toString();
                    if (i < arguments.size() - 1)
                        detail += ", ";
                }
                detail += ">";
            }
            return std::format("<NamedTypeExpr '{}'>", detail);
        }
    };

    struct NullableTypeExpr final : public TypeExpr
    {
        Expr *inner;

        NullableTypeExpr(Expr *_inner, SourceLocation _loc) : inner(_inner)
        {
            type     = AstType::NullableTypeExpr;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            return std::format("<NullableTypeExpr '{}?'>", inner->toString());
        }
    };

    struct FnTypeExpr final : public TypeExpr
    {
        // func (paratypes...) -> return_type

        DynArray<Expr *> paraTypes;
        Expr *returnType;

        FnTypeExpr(DynArray<Expr *> _paraTypes, Expr *_returnType) :
            paraTypes(std::move(_paraTypes)), returnType(_returnType)
        {
            type = AstType::FnTypeExpr;
        }

        virtual String toString() const override
        {
            String detail = "<FnTypeExpr 'func (";

            for (auto &pt : paraTypes)
            {
                if (pt != paraTypes.front())
                {
                    detail += ", ";
                }
                detail += pt->toString();
            }
            detail += ") -> ";
            detail += returnType->toString();
            detail += "'>";
            
            return detail;
        }
    };
} // namespace Fig
