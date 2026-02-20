/*!
    @file src/Compiler/ExprCompiler.cpp
    @brief 编译器实现(表达式部分)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Compiler/Compiler.hpp>

namespace Fig
{
    Result<std::uint8_t, Error> Compiler::CompileIdentiExpr(IdentiExpr *ie)
    {
        if (!HasLocal(ie->name))
        {
            return std::unexpected(Error(ErrorType::UseUndeclaredIdentifier,
                std::format("`{}` has not been defined", ie->name),
                "none",
                makeSourceLocation(ie)));
        }
        return ResolveLocal(ie->name);
    }
    Result<std::uint8_t, Error> Compiler::CompileLiteral(
        LiteralExpr *lit) // 编译字面量, 负责转换 token -> Value
    {
        const Token &token  = lit->token;
        String       lexeme = manager.GetSub(token.index, token.length);

        if (!token.isLiteral())
        {
            assert(false && "CompileLiteral: token is not literal");
        }

        Value v;

        if (token.type == TokenType::LiteralNull)
        {
            v = Value::GetNullInstance();
        }
        else if (token.type == TokenType::LiteralTrue)
        {
            v = Value::GetTrueInstance();
        }
        else if (token.type == TokenType::LiteralFalse)
        {
            v = Value::GetFalseInstance();
        }
        else if (token.type == TokenType::LiteralNumber)
        {
            // TODO: 更换为无异常手写数字解析版本
            if (lexeme.contains(U'.'))
            {
                // 非整数
                double d = std::stod(lexeme.toStdString());
                v        = Value::FromDouble(d);
            }
            std::int32_t i = std::stoi(lexeme.toStdString());
            v              = Value::FromInt(i);
        }
        else
        {
            assert("false" && "CompileLiteral: unsupport literal");
        }

        std::uint8_t  targetReg = AllocReg();
        std::uint16_t kIndex    = AddConstant(v);

        Emit(Op::iABx(OpCode::LoadK, targetReg, kIndex));
        return targetReg;
    }
    Result<std::uint8_t, Error> Compiler::CompileAssignment(
        InfixExpr *infix) // 编译赋值，由 CompileInfixExpr调用
    {
        // op必须为 =
        const auto &_lhsReg = CompileLeftValue(infix->left); // 必须为左值对象
        if (!_lhsReg)
        {
            return _lhsReg;
        }
        std::uint8_t lhsReg = *_lhsReg;

        const auto  &_rhsReg = CompileExpr(infix->right);
        std::uint8_t rhsReg  = *_rhsReg;

        FreeReg(rhsReg);
        switch (infix->op)
        {
            case BinaryOperator::Assign: {
                Emit(Op::iABx(OpCode::Mov, lhsReg, rhsReg)); // lhsReg = rhsReg
                break;
            }
            case BinaryOperator::AddAssign: {
                Emit(Op::iABC(OpCode::Add, lhsReg, lhsReg, rhsReg)); // lhsReg = lhsReg + rhsReg
                break;
            }
            case BinaryOperator::SubAssign: {
                Emit(Op::iABC(OpCode::Sub, lhsReg, lhsReg, rhsReg)); // lhsReg = lhsReg - rhsReg
                break;
            }
            case BinaryOperator::MultiplyAssign: {
                Emit(Op::iABC(OpCode::Mul, lhsReg, lhsReg, rhsReg)); // lhsReg = lhsReg * rhsReg
                break;
            }
            case BinaryOperator::DivideAssign: {
                Emit(Op::iABC(OpCode::Div, lhsReg, lhsReg, rhsReg)); // lhsReg = lhsReg / rhsReg
                break;
            }
            case BinaryOperator::ModuloAssign: {
                Emit(Op::iABC(OpCode::Mod, lhsReg, lhsReg, rhsReg)); // lhsReg = lhsReg % rhsReg
                break;
            }
            case BinaryOperator::BitXorAssign: {
                Emit(Op::iABC(OpCode::BitXor, lhsReg, lhsReg, rhsReg)); // lhsReg = lhsReg ^ rhsReg
                break;
            }
            default: {
                assert(false && "CompileAssignment: op unsupported yet");
            }
        }
        return lhsReg; // 返回赋值的结果，支持连续赋值
    }
    Result<std::uint8_t, Error> Compiler::CompileInfixExpr(
        InfixExpr *infix) // 编译中缀表达式，返回一个存放结果的寄存器 ID
    {
        if (infix->op >= BinaryOperator::Assign && infix->op <= BinaryOperator::BitXorAssign)
        {
            return CompileAssignment(infix);
        }

        const auto &_lhsReg = CompileExpr(infix->left);
        if (!_lhsReg)
        {
            return _lhsReg;
        }
        std::uint8_t lhsReg  = *_lhsReg;
        const auto  &_rhsReg = CompileExpr(infix->right);
        if (!_rhsReg)
        {
            return _rhsReg;
        }
        std::uint8_t rhsReg = *_rhsReg;

        FreeReg(rhsReg);
        FreeReg(lhsReg);

        std::uint8_t resultReg = AllocReg();
        switch (infix->op)
        {
            case BinaryOperator::Add: {
                Emit(Op::iABC(OpCode::Add, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Subtract: {
                Emit(Op::iABC(OpCode::Sub, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Multiply: {
                Emit(Op::iABC(OpCode::Mul, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Divide: {
                Emit(Op::iABC(OpCode::Div, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Modulo: {
                Emit(Op::iABC(OpCode::Mod, resultReg, lhsReg, rhsReg));
                break;
            }

            default: assert(false && "CompileInfixExpr: op unsupported yet");
        }
        return resultReg;
    }
    Result<std::uint8_t, Error> Compiler::CompileLeftValue(
        Expr *expr) // 左值对象，可以是变量、结构体字段或模块对象
    {
        switch (expr->type)
        {
            case AstType::IdentiExpr: return CompileIdentiExpr(static_cast<IdentiExpr *>(expr));

            default:
                return std::unexpected(Error(ErrorType::NotAnLvalue,
                    std::format("`{}` is not a lvalue, expect a valid lvalue", expr->toString()),
                    "none",
                    makeSourceLocation(expr)));
        }
    }
    Result<std::uint8_t, Error> Compiler::CompileExpr(
        Expr *expr) // 编译表达式，必定返回一个存放结果的寄存器 ID
    {
        switch (expr->type)
        {
            case AstType::Stmt:
            case AstType::Expr:
            case AstType::AstNode: assert(false && "CompileExpr: bad node type"); break;

            case AstType::IdentiExpr: {
                return CompileLeftValue(expr); // 左值直接转换成右值
            }
            case AstType::LiteralExpr: {
                LiteralExpr *lit = static_cast<LiteralExpr *>(expr);

                const auto &result = CompileLiteral(lit);
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                std::uint8_t targetReg = *result;
                return targetReg;
            }
            case AstType::InfixExpr: {
                return CompileInfixExpr(static_cast<InfixExpr *>(expr));
            }
        }
    }
} // namespace Fig