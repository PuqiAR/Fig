/*!
    @file src/Compiler/StmtCompiler.cpp
    @brief 编译器实现(语句部分)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Compiler/Compiler.hpp>

namespace Fig
{
    Result<void, Error> Compiler::compileVarDecl(VarDecl *varDecl)
    {
        if (current->freeReg > MAX_LOCALS)
        {
            return std::unexpected(Error(ErrorType::TooManyLocals,
                std::format("local limit exceeded: {}", MAX_LOCALS),
                "try split function or use arrays/structs...",
                makeSourceLocation(varDecl)));
        }

        std::uint8_t varReg;
        if (varDecl->initExpr)
        {
            auto result = compileExpr(varDecl->initExpr);
            if (!result)
            {
                return std::unexpected(result.error());
            }
            std::uint8_t resultReg = *result;
            varReg = DeclareLocal(varDecl->localId, resultReg); // 复用临时计算结果寄存器
        }
        else
        {
            // TODO: 未初始化提供初始值
            varReg = DeclareLocal(varDecl->localId);
        }

        return Result<void, Error>();
    }

    Result<void, Error> Compiler::compileBlockStmt(BlockStmt *blockStmt)
    {
        for (Stmt *stmt : blockStmt->nodes)
        {
            auto result = compileStmt(stmt);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    Result<void, Error> Compiler::compileIfStmt(IfStmt *stmt)
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
        const auto      &condResult = compileExpr(stmt->cond);
        if (!condResult)
        {
            return std::unexpected(condResult.error());
        }
        std::uint8_t condReg    = *condResult;
        int          jumpToNext = EmitJump(OpCode::JmpIfFalse, condReg);
        FreeReg(condReg);

        const auto &blockResult = compileStmt(stmt->consequent);
        if (!blockResult)
        {
            return blockResult;
        }

        exitJumps.push_back(EmitJump(OpCode::Jmp)); // 执行完if直接跳到出口
        PatchJump(jumpToNext);                      // 回填，跳到下一个else/elseif

        for (auto *elif : stmt->elifs)
        {
            const auto &elifCondResult = compileExpr(elif->cond);
            if (!elifCondResult)
                return std::unexpected(elifCondResult.error());
            std::uint8_t elifCondReg = *elifCondResult;

            jumpToNext = EmitJump(OpCode::JmpIfFalse, elifCondReg);
            FreeReg(elifCondReg);

            const auto &blockResult = compileStmt(elif->consequent);
            if (!blockResult)
            {
                return blockResult;
            }
            exitJumps.push_back(EmitJump(OpCode::Jmp)); // 执行完else if，跳到出口
            PatchJump(jumpToNext);                      // 跳到下一个分支
        }

        if (stmt->alternate)
        {
            auto result = compileStmt(stmt->alternate);
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

    Result<void, Error> Compiler::compileWhileStmt(WhileStmt *stmt)
    {
        int beginIns = current->proto->code.size() - 1;

        auto condRegResult = compileExpr(stmt->cond);
        if (!condRegResult)
        {
            return std::unexpected(condRegResult.error());
        }

        std::uint8_t condReg = *condRegResult;

        int  exitJump   = EmitJump(OpCode::JmpIfFalse, condReg);
        auto bodyResult = compileBlockStmt(stmt->body);

        if (!bodyResult)
        {
            return bodyResult;
        }
        Emit(Op::iAsBx(
            OpCode::Jmp, 0, beginIns - current->proto->code.size())); // 回到开头对condition求值
        PatchJump(exitJump);
        return {};
    }

    Result<void, Error> Compiler::compileFnDefStmt(FnDefStmt *stmt)
    {
        std::uint8_t funcReg = DeclareLocal(stmt->localId);

        // 创建子函数编译状态
        // 传入 current 作为 enclosing，用于后续支持闭包 Upvalue 查找
        FuncState childState(stmt->name, current);
        
        allProtos.push_back(childState.proto);
        std::uint16_t protoIdx = static_cast<std::uint16_t>(allProtos.size() - 1);

        globalFuncMap[stmt->localId] = protoIdx; // 把函数的local id映射到protoIdx

        {
            FuncStateProtector stateGuard(this, &childState);

            AllocReg();

            // 将参数映射为子函数的初始局部变量 (R0, R1...)
            for (Param *p : stmt->params)
            {
                PosParam *posParam = static_cast<PosParam *>(p); // TODO: 其他参数支持...
                // 按顺序分配寄存器
                DeclareLocal(posParam->localId);
            }

            // B编译函数体语句
            auto bodyRes = compileStmt(stmt->body);
            if (!bodyRes)
                return bodyRes;

            // 隐式返回 null
            std::uint8_t resReg = AllocReg();
            Emit(Op::iABC(OpCode::LoadNull, resReg, 0, 0));
            Emit(Op::iABC(OpCode::Return, resReg, 0, 0));
        }


        // 5. 检查是否是 main 函数
        if (stmt->name == U"main")
        {
            this->mainFuncIndex = protoIdx;
        }

        Emit(Op::iABx(OpCode::LoadFn, funcReg, protoIdx));

        return {};
    }

    Result<void, Error> Compiler::compileReturnStmt(ReturnStmt *stmt)
    {
        auto res = compileExpr(stmt->value);
        if (!res)
        {
            return std::unexpected(res.error());
        }

        Emit(Op::iABC(OpCode::Return, *res, 0, 0));
        return {};
    }

    Result<void, Error> Compiler::compileStmt(Stmt *stmt) // 编译语句
    {
        switch (stmt->type)
        {
            case AstType::ExprStmt: {
                ExprStmt *exprStmt = static_cast<ExprStmt *>(stmt);
                Expr     *expr     = exprStmt->expr;
                auto      result   = compileExpr(expr);
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                FreeReg(*result);
                break;
            }

            case AstType::VarDecl: {
                return compileVarDecl(static_cast<VarDecl *>(stmt));
            }

            case AstType::BlockStmt: {
                return compileBlockStmt(static_cast<BlockStmt *>(stmt));
            }

            case AstType::IfStmt: {
                return compileIfStmt(static_cast<IfStmt *>(stmt));
            }

            case AstType::WhileStmt: {
                return compileWhileStmt(static_cast<WhileStmt *>(stmt));
            }

            case AstType::FnDefStmt: {
                return compileFnDefStmt(static_cast<FnDefStmt *>(stmt));
            }
            
            case AstType::ReturnStmt: {
                return compileReturnStmt(static_cast<ReturnStmt *>(stmt));
            }
        }

        return Result<void, Error>();
    }
}; // namespace Fig