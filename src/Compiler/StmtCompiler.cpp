/*!
    @file src/Compiler/StmtCompiler.cpp
    @brief 语句编译器实现：实装水位线机制，彻底消灭硬编码寄存器释放
*/

#include <Ast/Stmt/FnDefStmt.hpp>
#include <Ast/Stmt/IfStmt.hpp>
#include <Ast/Stmt/VarDecl.hpp>
#include <Ast/Stmt/WhileStmt.hpp>
#include <Compiler/Compiler.hpp>


namespace Fig
{
    Result<void, Error> Compiler::compileStmt(Stmt *stmt)
    {
        if (stmt == nullptr)
        {
            return {};
        }

        switch (stmt->type)
        {
            case AstType::BlockStmt: {
                auto *b = static_cast<BlockStmt *>(stmt);
                for (auto *n : b->nodes)
                {
                    auto res = compileStmt(n);
                    if (!res)
                        return res;
                }
                break;
            }

            case AstType::VarDecl: {
                auto *v = static_cast<VarDecl *>(stmt);
                if (current->enclosing == nullptr) // 处理全局变量
                {
                    Register mark   = current->freereg; // 记录水位线
                    auto     regRes = compileExpr(v->initExpr);
                    if (!regRes)
                        return std::unexpected(regRes.error());

                    emit(Op::iABx(
                        OpCode::SetGlobal, *regRes, static_cast<uint16_t>(getGlobalID(v->name))));
                    current->freereg = mark; // 释放初始化表达式的临时占用
                }
                else
                {
                    // 抬升水位，锁死局部变量的物理槽位
                    Register targetReg = static_cast<Register>(v->localId);
                    while (current->freereg <= targetReg)
                    {
                        auto allocRes = allocateReg(v->location);
                        if (!allocRes)
                        {
                            return std::unexpected(allocRes.error());
                        }
                    }

                    auto regRes = compileExpr(v->initExpr, targetReg);
                    if (!regRes)
                        return std::unexpected(regRes.error());
                }
                break;
            }

            case AstType::FnDefStmt: {
                auto *f = static_cast<FnDefStmt *>(stmt);

                // 物理连线：对接 Compile() 第一阶段预分配的 Proto
                Proto *p = module->protos[f->resolvedSymbol->index];

                FuncState  fs(p, current);
                FuncState *old = current;
                current        = &fs;

                auto res = compileStmt(f->body);
                if (!res)
                    return res;

                // 窥孔拦截：防死代码污染
                if (p->code.empty() || static_cast<OpCode>(p->code.back() & 0xFF) != OpCode::Return)
                {
                    emit(Op::iABC(OpCode::Return, 0, 0, 0));
                }

                current = old;
                break;
            }

            case AstType::IfStmt: {
                auto         *i = static_cast<IfStmt *>(stmt);
                DynArray<int> exitJumps;

                Register mark   = current->freereg; // 记录水位线
                auto     r_cond = compileExpr(i->cond);
                if (!r_cond)
                    return std::unexpected(r_cond.error());

                int jmpToNext = static_cast<int>(current->proto->code.size());
                emit(Op::iAsBx(OpCode::JmpIfFalse, *r_cond, 0));
                current->freereg = mark; // 回收条件表达式临时槽位

                if (auto r = compileStmt(i->consequent); !r)
                    return r;
                exitJumps.push_back(static_cast<int>(current->proto->code.size()));
                emit(Op::iAsBx(OpCode::Jmp, 0, 0));

                int targetIdx                   = static_cast<int>(current->proto->code.size());
                current->proto->code[jmpToNext] = Op::iAsBx(
                    OpCode::JmpIfFalse, *r_cond, static_cast<int16_t>(targetIdx - jmpToNext - 1));

                for (auto *elif : i->elifs)
                {
                    Register elifMark = current->freereg;
                    auto     ec       = compileExpr(elif->cond);
                    if (!ec)
                        return std::unexpected(ec.error());

                    int nextElif = static_cast<int>(current->proto->code.size());
                    emit(Op::iAsBx(OpCode::JmpIfFalse, *ec, 0));
                    current->freereg = elifMark; // 回收 elif 临时槽位

                    if (auto r = compileStmt(elif->consequent); !r)
                        return r;
                    exitJumps.push_back(static_cast<int>(current->proto->code.size()));
                    emit(Op::iAsBx(OpCode::Jmp, 0, 0));

                    int target                     = static_cast<int>(current->proto->code.size());
                    current->proto->code[nextElif] = Op::iAsBx(
                        OpCode::JmpIfFalse, *ec, static_cast<int16_t>(target - nextElif - 1));
                }

                if (i->alternate)
                {
                    if (auto r = compileStmt(i->alternate); !r)
                        return r;
                }

                int endIdx = static_cast<int>(current->proto->code.size());
                for (int pos : exitJumps)
                {
                    current->proto->code[pos] =
                        Op::iAsBx(OpCode::Jmp, 0, static_cast<int16_t>(endIdx - pos - 1));
                }
                break;
            }

            case AstType::WhileStmt: {
                auto *w        = static_cast<WhileStmt *>(stmt);
                int   startIdx = static_cast<int>(current->proto->code.size());

                Register mark   = current->freereg; // 记录水位线
                auto     r_cond = compileExpr(w->cond);
                if (!r_cond)
                    return std::unexpected(r_cond.error());

                int exitJmpIdx = static_cast<int>(current->proto->code.size());
                emit(Op::iAsBx(OpCode::JmpIfFalse, *r_cond, 0));
                current->freereg = mark; // 回收循环条件临时槽位

                if (auto r = compileStmt(w->body); !r)
                    return r;

                int backJmpIdx = static_cast<int>(current->proto->code.size());
                emit(Op::iAsBx(OpCode::Jmp, 0, static_cast<int16_t>(startIdx - backJmpIdx - 1)));

                int endIdx                       = static_cast<int>(current->proto->code.size());
                current->proto->code[exitJmpIdx] = Op::iAsBx(
                    OpCode::JmpIfFalse, *r_cond, static_cast<int16_t>(endIdx - exitJmpIdx - 1));
                break;
            }

            case AstType::ReturnStmt: {
                auto    *rs   = static_cast<ReturnStmt *>(stmt);
                Register mark = current->freereg; // 记录水位线
                Register retReg;

                if (rs->value)
                {
                    auto r = compileExpr(rs->value);
                    if (!r)
                        return std::unexpected(r.error());
                    retReg = *r;
                }
                else
                {
                    auto r = allocateReg(rs->location);
                    if (!r)
                        return std::unexpected(r.error());
                    emit(Op::iABC(OpCode::LoadNull, *r, 0, 0));
                    retReg = *r;
                }

                emit(Op::iABC(OpCode::Return, retReg, 0, 0));
                current->freereg = mark; // 回收返回值计算的占用
                break;
            }

            case AstType::ExprStmt: {
                Register mark = current->freereg; // 记录水位线
                auto     reg  = compileExpr(static_cast<ExprStmt *>(stmt)->expr);
                if (!reg)
                    return std::unexpected(reg.error());

                current->freereg = mark; // 彻底抛弃孤立表达式的副作用
                break;
            }

            default: break;
        }
        return {};
    }
} // namespace Fig