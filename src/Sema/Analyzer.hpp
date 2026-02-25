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
        Environment env;
        SourceManager &manager;

        SourceLocation makeSourceLocation(AstNode *ast, std::source_location loc = std::source_location::current())
        {
            return SourceLocation(
                ast->location.sp,
                ast->location.fileName,
                "[internal analyzer]",
                loc.function_name()
            );
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

        Result<void, Error> analyzeVarDecl(VarDecl *);
        Result<void, Error> analyzeIfStmt(IfStmt *);
        Result<void, Error> analyzeWhileStmt(WhileStmt *);
        
        Result<void, Error> analyzeIdentiExpr(IdentiExpr *);
        Result<void, Error> analyzeInfixExpr(InfixExpr *);

        Result<void, Error> analyzeStmt(Stmt *);
        Result<void, Error> analyzeExpr(Expr *);
    public:
        Result<void, Error> Analyze(Program *);

        Analyzer(SourceManager &_manager) : manager(_manager) {}
    };
}; // namespace Fig