/*!
    @file src/Sema/Analyzer.hpp
    @brief 前端类型检查器定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-23
*/

#pragma once

#include <Sema/Environment.hpp>
#include <Sema/Type.hpp>

#include <Ast/Ast.hpp>
#include <Deps/Deps.hpp>

namespace Fig
{
    class Analyzer
    {
    private:
        Environment    env;
        SourceManager &manager;

        TypeContext typeCtx;

        TypeInfo *currentReturnType = nullptr; // 正在分析的函数，预期返回类型

        struct ReturnTypeProtector
        {
            Analyzer *analyzer;
            TypeInfo *prevCurrentReturnType;

            [[nodiscard]]
            ReturnTypeProtector(Analyzer *_analyzer, TypeInfo *current) :
                analyzer(_analyzer), prevCurrentReturnType(_analyzer->currentReturnType)
            {
                analyzer->currentReturnType = current;
            }

            ~ReturnTypeProtector()
            {
                analyzer->currentReturnType = prevCurrentReturnType;
            }

            ReturnTypeProtector(const ReturnTypeProtector &) = delete;
            ReturnTypeProtector &operator=(const ReturnTypeProtector &) = delete;
        };


        SourceLocation makeSourceLocation(
            AstNode *ast, std::source_location loc = std::source_location::current())
        {
            return SourceLocation(ast->location.sp,
                ast->location.fileName,
                "[internal analyzer]",
                loc.function_name());
        }

        bool isValidLvalue(Expr *expr)
        {
            if (expr->type == AstType::IdentiExpr)
            {
                return true;
            }
            if (expr->type == AstType::InfixExpr)
            {
                InfixExpr *infix = static_cast<InfixExpr *>(expr);
                if (infix->op == BinaryOperator::MemberAccess)
                {
                    return true;
                }
            }
            if (expr->type == AstType::IndexExpr)
            {
                return true;
            }
            return false;
        }

        Result<TypeInfo *, Error> resolveType(TypeExpr *);

        Result<void, Error> analyzeVarDecl(VarDecl *);
        Result<void, Error> analyzeIfStmt(IfStmt *);
        Result<void, Error> analyzeWhileStmt(WhileStmt *);
        Result<void, Error> analyzeFnDefStmt(FnDefStmt *);
        Result<void, Error> analyzeReturnStmt(ReturnStmt *);

        Result<void, Error> analyzeIdentiExpr(IdentiExpr *);
        Result<void, Error> analyzeInfixExpr(InfixExpr *);

        Result<void, Error> analyzeStmt(Stmt *);
        Result<void, Error> analyzeExpr(Expr *);

    public:
        Result<void, Error> Analyze(Program *);

        Analyzer(SourceManager &_manager) : manager(_manager), typeCtx() {}
    };
}; // namespace Fig