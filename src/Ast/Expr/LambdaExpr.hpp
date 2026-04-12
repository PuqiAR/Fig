/*!
    @file src/Ast/Expr/LambdaExpr.hpp
    @brief Lambda表达式定义
*/

#pragma once

#include <Ast/Base.hpp>
#include <Ast/Stmt/FnDefStmt.hpp>

namespace Fig
{
    struct LambdaExpr final : public Expr
    {
        // func (params) [-> return type] ([=> expr] / [ {stmt} ])

        DynArray<Param *> params;
        TypeExpr         *returnType;
        AstNode          *body; // expr/blockstmt
        bool              isExprBody;

        DynArray<UpvalueInfo> upvalues;

        LambdaExpr()
        {
            type = AstType::LambdaExpr;
        }

        LambdaExpr(
            DynArray<Param *> _params,
            TypeExpr         *_returnType,
            AstNode          *_body,
            bool              _isExprBody,
            SourceLocation    _location) :
            params(std::move(_params)),
            returnType(_returnType),
            body(_body),
            isExprBody(_isExprBody)
        {
            type     = AstType::LambdaExpr;
            location = std::move(_location);
        }

        virtual String toString() const override
        {
            String specifying = "<LambdaExpr 'func (";
            for (auto &p : params)
            {
                if (p != params.front())
                {
                    specifying += ", ";
                }
                specifying += p->toString();
            }
            if (isExprBody)
            {
                specifying += ") => ";
                specifying += body->toString();
            }
            else
            {
                specifying += ") {";
                specifying += body->toString();
                specifying.push_back(U'}');
            }
            specifying += "'>";
            return specifying;
        }
    };
}; // namespace Fig