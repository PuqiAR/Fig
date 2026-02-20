/*!
    @file src/Compiler/StmtCompiler.cpp
    @brief 编译器实现(语句部分)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Compiler/Compiler.hpp>

namespace Fig
{
    Result<void, Error> Compiler::CompileVarDecl(VarDecl *varDecl)
    {
        const String &name = varDecl->name;
        if (HasLocalInCurrentScope(name))
        {
            return std::unexpected(Error(ErrorType::RedeclarationError,
                std::format("variable `{}` has already defined in this scope", name),
                "change its name",
                makeSourceLocation(varDecl)));
        }
        std::uint8_t varReg;
        if (varDecl->initExpr)
        {
            const auto &result = CompileExpr(varDecl->initExpr);
            if (!result)
            {
                return std::unexpected(result.error());
            }
            std::uint8_t resultReg = *result;
            varReg                 = resultReg; // 复用临时计算结果寄存器
            DeclareLocal(varDecl->isPublic, name, varReg);
        }
        else
        {
            varReg = DeclareLocal(varDecl->isPublic, name);
        }
        return Result<void, Error>();
    }
    Result<void, Error> Compiler::CompileStmt(Stmt *stmt) // 编译语句
    {
        if (stmt->type == AstType::ExprStmt)
        {
            ExprStmt   *exprStmt = static_cast<ExprStmt *>(stmt);
            Expr       *expr     = exprStmt->expr;
            const auto &result   = CompileExpr(expr);
            if (!result)
            {
                return std::unexpected(result.error());
            }
        }
        else if (stmt->type == AstType::VarDecl)
        {
            return CompileVarDecl(static_cast<VarDecl *>(stmt));
        }
        return Result<void, Error>();
    }
}; // namespace Fig