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

    Result<void, Error> Compiler::CompileBlockStmt(BlockStmt *blockStmt)
    {
        for (Stmt *stmt : blockStmt->nodes)
        {
            const auto &result = CompileStmt(stmt);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    Result<void, Error> Compiler::CompileIfStmt(IfStmt *stmt)
    {
        /*
            if cond1
            {
            }
            else if cond2   #1
            {
            }
            else if cond3   #2
            {
            }
            else            #3
            {
            }
            quit #4

            Bytecode:
                JmpIfFalse cond1 #1
                ; consequent内容
                ; ...
                ; if条件为true, 跳过所有 else/elseif

                Jmp #4

                ; #1
                JmpIfFalse cond2 #2
                ; consequent

                Jmp #4

                ; #2
                JmpIfFalse cond3 #3
                ; consequent

                Jmp #4

                ; #3
                ; 没有一次执行分支
                ; else部分
                ; ...

                #4

        */
        std::vector<int> exitJumps; // 所有分支都要跳到最后，收集所有jump最后回填
        const auto      &condResult = CompileExpr(stmt->cond);
        if (!condResult)
        {
            return std::unexpected(condResult.error());
        }
        std::uint8_t condReg    = *condResult;
        int          jumpToNext = EmitJump(OpCode::JmpIfFalse, condReg);
        FreeReg(condReg);

        const auto &blockResult = CompileStmt(stmt->consequent);
        if (!blockResult)
        {
            return blockResult;
        }

        exitJumps.push_back(EmitJump(OpCode::Jmp)); // 执行完if直接跳到出口
        PatchJump(jumpToNext);                      // 回填，跳到下一个else/elseif

        for (auto *elif : stmt->elifs)
        {
            const auto &elifCondResult = CompileExpr(elif->cond);
            if (!elifCondResult)
                return std::unexpected(elifCondResult.error());
            std::uint8_t elifCondReg = *elifCondResult;

            jumpToNext = EmitJump(OpCode::JmpIfFalse, elifCondReg);
            FreeReg(elifCondReg);

            const auto &blockResult = CompileStmt(elif->consequent);
            if (!blockResult)
            {
                return blockResult;
            }
            exitJumps.push_back(EmitJump(OpCode::Jmp)); // 执行完else if，跳到出口
            PatchJump(jumpToNext);                      // 跳到下一个分支
        }

        if (stmt->alternate)
        {
            const auto &result = CompileStmt(stmt->alternate);
            if (!result)
            {
                return result;
            }
        }
        for (int exitIndex : exitJumps)
        {
            PatchJump(exitIndex); // 回填所有跳转出口的指令
        }
        return {};
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
            FreeReg(*result);
        }
        else if (stmt->type == AstType::VarDecl)
        {
            return CompileVarDecl(static_cast<VarDecl *>(stmt));
        }
        else if (stmt->type == AstType::BlockStmt)
        {
            return CompileBlockStmt(static_cast<BlockStmt *>(stmt));
        }
        else if (stmt->type == AstType::IfStmt)
        {
            return CompileIfStmt(static_cast<IfStmt *>(stmt));
        }
        return Result<void, Error>();
    }
}; // namespace Fig