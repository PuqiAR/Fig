/*!
    @file src/Compiler/ExprCompiler.cpp
    @brief 表达式编译器实现：引入水位线(Watermark)与零拷贝复用机制
*/

#include <Ast/Expr/CallExpr.hpp>
#include <Ast/Expr/IdentiExpr.hpp>
#include <Ast/Expr/InfixExpr.hpp>
#include <Ast/Expr/LiteralExpr.hpp>
#include <Compiler/Compiler.hpp>
#include <charconv>
#include <limits>
#include <system_error>


namespace Fig
{
    static Result<Value, Error> parsePhysicalNumber(const String &raw, const SourceLocation &loc)
    {
        char buffer[128];
        int  j       = 0;
        bool isFloat = false;
        for (size_t i = 0; i < raw.length() && j < 127; ++i)
        {
            char32_t c = raw[i];
            if (c == '_')
                continue;
            if (c == '.' || c == 'e' || c == 'E')
                isFloat = true;
            buffer[j++] = (char) c;
        }
        buffer[j] = '\0';

        if (isFloat)
        {
            double dVal;
            auto [ptr, ec] = std::from_chars(buffer, buffer + j, dVal);
            if (ec != std::errc())
                return std::unexpected(Error(ErrorType::SyntaxError, "float overflow", "", loc));
            return Value::FromDouble(dVal);
        }
        else
        {
            int         base  = 10;
            const char *start = buffer;
            if (j > 2 && buffer[0] == '0')
            {
                if (buffer[1] == 'x' || buffer[1] == 'X')
                {
                    base = 16;
                    start += 2;
                }
                else if (buffer[1] == 'b' || buffer[1] == 'B')
                {
                    base = 2;
                    start += 2;
                }
            }
            int64_t iVal;
            auto [ptr, ec] = std::from_chars(start, buffer + j, iVal, base);
            if (ec != std::errc())
                return std::unexpected(Error(ErrorType::SyntaxError, "integer overflow", "", loc));

            if (iVal >= std::numeric_limits<int32_t>::min()
                && iVal <= std::numeric_limits<int32_t>::max())
            {
                return Value::FromInt(static_cast<int32_t>(iVal));
            }
            else
            {
                return Value::FromDouble(static_cast<double>(iVal));
            }
        }
    }

    Result<Register, Error> Compiler::compileExpr(Expr *expr, Register target)
    {
        if (expr == nullptr)
        {
            return std::unexpected(
                Error(ErrorType::InternalError, "null expr in compiler", "", {}));
        }

        switch (expr->type)
        {
            case AstType::LiteralExpr: {
                auto    *l = static_cast<LiteralExpr *>(expr);
                Register r = (target == NO_REG) ? *allocateReg(l->location) : target;

                const Token &tok = l->literal;
                if (tok.type == TokenType::LiteralNumber)
                {
                    auto vRes =
                        parsePhysicalNumber(manager.GetSub(tok.index, tok.length), l->location);
                    if (!vRes)
                        return std::unexpected(vRes.error());
                    emit(Op::iABx(OpCode::LoadK, r, static_cast<uint16_t>(addConstant(*vRes))), &l->location);
                }
                else if (tok.type == TokenType::LiteralString)
                {
                    int kIdx = addConstant(Value::GetNullInstance()); // TODO: String 支持
                    emit(Op::iABx(OpCode::LoadK, r, static_cast<uint16_t>(kIdx)), &l->location);
                }
                else if (tok.type == TokenType::LiteralNull)
                {
                    emit(Op::iABC(OpCode::LoadNull, r, 0, 0), &l->location);
                }
                else if (tok.type == TokenType::LiteralTrue)
                {
                    emit(Op::iABC(OpCode::LoadTrue, r, 0, 0), &l->location);
                }
                else if (tok.type == TokenType::LiteralFalse)
                {
                    emit(Op::iABC(OpCode::LoadFalse, r, 0, 0), &l->location);
                }
                return r;
            }

            case AstType::IdentiExpr: {
                auto   *i   = static_cast<IdentiExpr *>(expr);
                Symbol *sym = i->resolvedSymbol;

                if (sym->location == SymbolLocation::Local)
                {
                    // 零拷贝直读：如果是临时求值，直接返回变量的物理槽位，禁止产生副本
                    if (target == NO_REG)
                        return static_cast<Register>(sym->index);

                    // 仅在被强制指定目标（如参数装填）时发射搬运指令
                    if (target != sym->index)
                    {
                        emit(Op::iABx(OpCode::Mov, target, static_cast<uint16_t>(sym->index)), &i->location);
                    }
                    return target;
                }

                Register r = (target == NO_REG) ? *allocateReg(i->location) : target;
                if (sym->location == SymbolLocation::Upvalue)
                {
                    emit(Op::iABC(OpCode::GetUpval, r, static_cast<uint8_t>(sym->index), 0), &i->location);
                }
                else if (sym->location == SymbolLocation::Global)
                {
                    int gId = getGlobalID(i->name);
                    emit(Op::iABx(OpCode::GetGlobal, r, static_cast<uint16_t>(gId)), &i->location);
                }
                return r;
            }

            case AstType::CallExpr: {
                auto    *c       = static_cast<CallExpr *>(expr);
                Register mark    = current->freereg; // 记录调用前的栈顶水位
                Register baseReg = current->freereg; // 锁定滑窗基址

                // 连续装填参数，占据 baseReg, baseReg+1, baseReg+2...
                for (auto *arg : c->args.args)
                {
                    auto allocRes = allocateReg(arg->location);
                    if (!allocRes)
                    {
                        return allocRes;
                    }

                    Register argTarget = *allocRes;
                    auto     res       = compileExpr(arg, argTarget);
                    if (!res)
                        return std::unexpected(res.error());
                }

                bool isGlobalFastCall = false;
                if (c->callee->type == AstType::IdentiExpr)
                {
                    auto *id = static_cast<IdentiExpr *>(c->callee);
                    // 只有在全局区的函数，才能使用 FastCall
                    if (id->resolvedSymbol->location == SymbolLocation::Global)
                    {
                        isGlobalFastCall = true;
                        int protoIdx     = id->resolvedSymbol->index;
                        emit(Op::iABC(OpCode::FastCall,
                                 static_cast<uint8_t>(protoIdx),
                                 baseReg,
                                 static_cast<uint8_t>(c->args.args.size())),
                            &c->location);
                    }
                }

                if (!isGlobalFastCall)
                {
                    // 动态闭包调用
                    // 先获取闭包对象所在的物理寄存器
                    auto r_fn = compileExpr(c->callee);
                    if (!r_fn)
                        return std::unexpected(r_fn.error());

                    // 使用动态 Call 指令，RA 是指向堆闭包的寄存器
                    emit(Op::iABC(OpCode::Call,
                             *r_fn,
                             baseReg,
                             static_cast<uint8_t>(c->args.args.size())),
                        &c->location);
                }

                // 回滚水位线, 释放传参时的临时占用
                current->freereg = mark;

                // 目若 target 未指定，allocateReg 将复用 baseReg，实现零开销回写

                Register r_dest;
                if (target == NO_REG)
                {
                    auto res = allocateReg(c->location);
                    if (!res)
                        return std::unexpected(res.error());
                    r_dest = *res;
                }
                else
                {
                    r_dest = target;
                }

                if (r_dest != baseReg)
                {
                    emit(Op::iABx(OpCode::Mov, r_dest, baseReg), &c->location);
                }

                return r_dest;
            }

            case AstType::InfixExpr: {
                auto *in = static_cast<InfixExpr *>(expr);
                if (in->op == BinaryOperator::Assign)
                {
                    auto r_val = compileExpr(in->right, target);
                    if (!r_val)
                        return std::unexpected(r_val.error());

                    if (in->left->type == AstType::IdentiExpr)
                    {
                        auto   *lid = static_cast<IdentiExpr *>(in->left);
                        Symbol *sym = lid->resolvedSymbol;
                        if (sym->location == SymbolLocation::Local)
                        {
                            emit(Op::iABx(OpCode::Mov, static_cast<Register>(sym->index), *r_val), &lid->location);
                        }
                        else if (sym->location == SymbolLocation::Upvalue)
                        {
                            emit(Op::iABC(
                                OpCode::SetUpval, *r_val, static_cast<Register>(sym->index), 0), &lid->location);
                        }
                        else
                        {
                            emit(Op::iABx(OpCode::SetGlobal,
                                *r_val,
                                static_cast<uint16_t>(getGlobalID(lid->name))), &lid->location);
                        }
                    }
                    return r_val;
                }

                Register mark = current->freereg; // 记录水位线

                auto r_l = compileExpr(in->left);
                if (!r_l)
                    return std::unexpected(r_l.error());
                auto r_r = compileExpr(in->right);
                if (!r_r)
                    return std::unexpected(r_r.error());

                bool isInt = in->left->resolvedType.is(TypeTag::Int)
                             && in->right->resolvedType.is(TypeTag::Int);
                OpCode op;
                switch (in->op)
                {
                    case BinaryOperator::Add: op = isInt ? OpCode::IntFastAdd : OpCode::Add; break;
                    case BinaryOperator::Subtract:
                        op = isInt ? OpCode::IntFastSub : OpCode::Sub;
                        break;
                    case BinaryOperator::Multiply:
                        op = isInt ? OpCode::IntFastMul : OpCode::Mul;
                        break;
                    case BinaryOperator::Divide:
                        op = isInt ? OpCode::IntFastDiv : OpCode::Div;
                        break;
                    case BinaryOperator::Modulo: op = OpCode::Mod; break;
                    case BinaryOperator::BitXor: op = OpCode::BitXor; break;
                    case BinaryOperator::Equal: op = OpCode::Equal; break;
                    case BinaryOperator::NotEqual: op = OpCode::NotEqual; break;
                    case BinaryOperator::Greater: op = OpCode::Greater; break;
                    case BinaryOperator::Less: op = OpCode::Less; break;
                    case BinaryOperator::GreaterEqual: op = OpCode::GreaterEqual; break;
                    case BinaryOperator::LessEqual: op = OpCode::LessEqual; break;
                    default:
                        return std::unexpected(Error(ErrorType::InternalError,
                            "unsupported binary operator",
                            "",
                            in->location));
                }

                // 释放左右操作数产生的临时寄存器
                current->freereg = mark;

                // 复用已释放的物理槽位存放计算结果
                Register r_d;
                if (target == NO_REG)
                {
                    auto res = allocateReg(in->location);
                    if (!res)
                        return std::unexpected(res.error());
                    r_d = *res;
                }
                else
                {
                    r_d = target;
                }

                emit(Op::iABC(op, r_d, *r_l, *r_r), &in->location);

                return r_d;
            }

            default: break;
        }
        return std::unexpected(
            Error(ErrorType::InternalError, "unsupported expr", "", expr->location));
    }
} // namespace Fig