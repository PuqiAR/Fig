/*!
    @file src/Compiler/ExprCompiler.cpp
    @brief 编译器实现(表达式部分)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Compiler/Compiler.hpp>

namespace Fig
{
    Result<std::uint8_t, Error> Compiler::compileIdentiExpr(IdentiExpr *ie)
    {
        // TODO: 处理全局变量和闭包 Upvalue
        std::uint8_t targetReg = current->fastRegMap[ie->localId];

        if (targetReg == UINT8_MAX)
        {
            assert(false && "Compiler Bug: Encountered unmapped localId in fastRegMap!");
        }
        return targetReg;
    }
    Result<std::uint8_t, Error> Compiler::compileLiteral(
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
            // TODO: 更换为无异常手写数字解析版本 (charconv也可)
            if (lexeme.contains(U'.') || lexeme.contains(U'e'))
            {
                // 非整数
                double d = std::stod(lexeme.toStdString());
                v        = Value::FromDouble(d);
            }
            else
            {
                std::int32_t i = std::stoi(lexeme.toStdString());
                v              = Value::FromInt(i);
            }
        }
        else
        {
            assert("false" && "CompileLiteral: unsupport literal");
        }

        std::uint8_t targetReg = AllocReg();

        if (current->proto->constants.size() >= MAX_CONSTANTS)
        {
            return std::unexpected(Error(ErrorType::TooManyConstants,
                std::format("constant limit exceeded: {}", MAX_CONSTANTS),
                "How did you write such code? try global variable or split file",
                makeSourceLocation(lit)));
        }

        std::uint16_t kIndex = AddConstant(v);

        Emit(Op::iABx(OpCode::LoadK, targetReg, kIndex));
        return targetReg;
    }
    Result<std::uint8_t, Error> Compiler::compileAssignment(
        InfixExpr *infix) // 编译赋值，由 CompileInfixExpr调用
    {
        // op必须为 =
        const auto &_lhsReg = compileLeftValue(infix->left); // 必须为左值对象
        if (!_lhsReg)
        {
            return _lhsReg;
        }
        std::uint8_t lhsReg = *_lhsReg;

        const auto  &_rhsReg = compileExpr(infix->right);
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
    Result<std::uint8_t, Error> Compiler::compileInfixExpr(
        InfixExpr *infix) // 编译中缀表达式，返回一个存放结果的寄存器 ID
    {
        if (infix->op >= BinaryOperator::Assign && infix->op <= BinaryOperator::BitXorAssign)
        {
            return compileAssignment(infix);
        }

        Expr *left  = infix->left;
        Expr *right = infix->right;

        const auto &_lhsReg = compileExpr(left);
        if (!_lhsReg)
        {
            return _lhsReg;
        }
        std::uint8_t lhsReg  = *_lhsReg;
        const auto  &_rhsReg = compileExpr(right);
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
                if (left->resolvedType == right->resolvedType && left->resolvedType->isInt())
                {
                    // Int + Int
                    Emit(Op::iABC(OpCode::IntFastAdd, resultReg, lhsReg, rhsReg));
                }
                else
                {
                    Emit(Op::iABC(OpCode::Add, resultReg, lhsReg, rhsReg));
                }
                break;
            }

            case BinaryOperator::Subtract: {
                if (left->resolvedType == right->resolvedType && left->resolvedType->isInt())
                {
                    // Int - Int
                    Emit(Op::iABC(OpCode::IntFastSub, resultReg, lhsReg, rhsReg));
                }
                else
                {
                    Emit(Op::iABC(OpCode::Sub, resultReg, lhsReg, rhsReg));
                }
                break;
            }

            case BinaryOperator::Multiply: {
                if (left->resolvedType == right->resolvedType && left->resolvedType->isInt())
                {
                    // Int * Int
                    Emit(Op::iABC(OpCode::IntFastMul, resultReg, lhsReg, rhsReg));
                }
                else
                {
                    Emit(Op::iABC(OpCode::Mul, resultReg, lhsReg, rhsReg));
                }
                break;
            }

            case BinaryOperator::Divide: {
                if (left->resolvedType == right->resolvedType && left->resolvedType->isInt())
                {
                    // Int / Int
                    Emit(Op::iABC(OpCode::IntFastDiv, resultReg, lhsReg, rhsReg));
                }
                else
                {
                    Emit(Op::iABC(OpCode::Div, resultReg, lhsReg, rhsReg));
                }
                break;
            }

            case BinaryOperator::Modulo: {
                Emit(Op::iABC(OpCode::Mod, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Greater: {
                Emit(Op::iABC(OpCode::Greater, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::GreaterEqual: {
                Emit(Op::iABC(OpCode::GreaterEqual, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Less: {
                Emit(Op::iABC(OpCode::Less, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::LessEqual: {
                Emit(Op::iABC(OpCode::LessEqual, resultReg, lhsReg, rhsReg));
                break;
            }

            case BinaryOperator::Equal: {
                Emit(Op::iABC(OpCode::Equal, resultReg, lhsReg, rhsReg));
                break;
            }

            default: assert(false && "CompileInfixExpr: op unsupported yet");
        }
        return resultReg;
    }
    Result<std::uint8_t, Error> Compiler::compileLeftValue(
        Expr *expr) // 左值对象，可以是变量、结构体字段或模块对象
    {
        switch (expr->type)
        {
            case AstType::IdentiExpr:
                return compileIdentiExpr(static_cast<IdentiExpr *>(expr));
                // TODO: 数组切片(a[0])或对象属性(a.b)

            default:
                // Analyzer 有漏洞（编译器内部
                // 直接崩溃
                assert(false && "Compiler Bug: Invalid L-value bypassed Analyzer!");
                return 0;
        }
    }

    Result<std::uint8_t, Error> Compiler::compileCallExpr(CallExpr *expr)
    {
        bool isStatic = false; // 是否为单纯的 fn(...) 静态函数调用
        int  protoIdx = -1;

        if (expr->callee->type == AstType::IdentiExpr)
        {
            IdentiExpr *id = static_cast<IdentiExpr *>(expr->callee);
            // 如果是函数名且深度为 0 (全局/扁平函数池)
            if (id->resolvedType->tag == TypeTag::Function && id->resolvedDepth == 0)
            {
                if (globalFuncMap.contains(id->localId))
                {
                    isStatic = true;
                    protoIdx = globalFuncMap[id->localId];
                }
            }
        }

        std::uint8_t baseReg = AllocReg();

        if (!isStatic)
        {
            auto calleeRes = compileExpr(expr->callee);
            if (!calleeRes)
            {
                return calleeRes;
            }

            if (*calleeRes != baseReg)
            {
                Emit(Op::iABx(OpCode::Mov, baseReg, *calleeRes));
            }
        }

        for (size_t i = 0; i < expr->args.size(); ++i)
        {
            std::uint8_t argTarget = AllocReg();
            auto         argRes    = compileExpr(expr->args.args[i]);
            if (!argRes)
            {
                return argRes;
            }

            if (*argRes != argTarget)
            {
                Emit(Op::iABx(OpCode::Mov, argTarget, *argRes));
            }
        }

        std::uint8_t expectRet = 1;

        if (isStatic)
        {
            Emit(Op::iABC(OpCode::FastCall, (std::uint8_t) protoIdx, baseReg, expectRet));
        }
        else
        {
            Emit(Op::iABC(OpCode::Call, baseReg, baseReg, expectRet));
        }

        for (size_t i = 0; i < expr->args.args.size(); ++i)
        {
            current->freeReg--;
        }
        return baseReg; // 返回值起点
    }

    Result<std::uint8_t, Error> Compiler::compileExpr(
        Expr *expr) // 编译表达式，必定返回一个存放结果的寄存器 ID
    {
        switch (expr->type)
        {
            case AstType::Stmt:
            case AstType::Expr:
            case AstType::AstNode: assert(false && "CompileExpr: bad node type"); break;

            case AstType::IdentiExpr: {
                return compileLeftValue(expr); // 左值直接转换成右值
            }

            case AstType::LiteralExpr: {
                LiteralExpr *lit = static_cast<LiteralExpr *>(expr);

                auto result = compileLiteral(lit);
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                std::uint8_t targetReg = *result;
                return targetReg;
            }

            case AstType::InfixExpr: {
                return compileInfixExpr(static_cast<InfixExpr *>(expr));
            }

            case AstType::CallExpr: {
                return compileCallExpr(static_cast<CallExpr *>(expr));
            }
        }
    }
} // namespace Fig