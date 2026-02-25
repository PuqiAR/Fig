/*!
    @file src/Sema/Analyzer.hpp
    @brief 前端类型检查器实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-23
*/

#include <Sema/Analyzer.hpp>

namespace Fig
{
    Result<void, Error> Analyzer::analyzeVarDecl(VarDecl *stmt)
    {
        auto sym = env.Resolve(stmt->name);
        if (sym != std::nullopt && sym->depth == env.GetDepth())
        {
            return std::unexpected(Error(ErrorType::RedeclarationError,
                std::format("variable `{}` has already defined in this scope", stmt->name),
                "change its name",
                makeSourceLocation(stmt)));
        }

        TypeTag initType = TypeTag::Any;
        if (stmt->initExpr)
        {
            const auto &res = analyzeExpr(stmt->initExpr);
            if (!res)
            {
                return res;
            }
            initType = stmt->initExpr->resolvedType;
        }

        TypeTag declaredType = TypeTag::Any;
        if (stmt->typeSpecifier)
        {
            // TODO: 解析类型定义
        }

        if (stmt->isInfer)
        {
            declaredType = initType;
        }
        else if (declaredType != TypeTag::Any && declaredType != initType)
        {
            return std::unexpected(Error(ErrorType::TypeError,
                std::format("cannot assign type `{}` to variable {} which speicifer type is '{}'",
                    magic_enum::enum_name(initType),
                    stmt->name,
                    magic_enum::enum_name(declaredType)),
                "none",
                makeSourceLocation(stmt->initExpr)));
        }
        stmt->localId = env.Define(stmt->name, declaredType, stmt->isPublic, false);
        return {};
    }

    Result<void, Error> Analyzer::analyzeIfStmt(IfStmt *stmt)
    {
        auto condRes = analyzeExpr(stmt->cond);
        if (!condRes)
        {
            return condRes;
        }
        if (stmt->cond->resolvedType != TypeTag::Any && stmt->cond->resolvedType != TypeTag::Bool)
        {
            return std::unexpected(Error(ErrorType::TypeError,
                std::format("if condition must be boolean, got `{}`",
                    magic_enum::enum_name(stmt->cond->resolvedType)),
                "ensure condition is boolean",
                makeSourceLocation(stmt->cond)));
        }
        auto consequentRes = analyzeStmt(stmt->consequent);
        if (!consequentRes)
        {
            return consequentRes;
        }

        for (ElseIfStmt *elif : stmt->elifs)
        {
            auto condRes = analyzeExpr(elif->cond);
            if (elif->cond->resolvedType != TypeTag::Any
                && elif->cond->resolvedType != TypeTag::Bool)
            {
                return std::unexpected(Error(ErrorType::TypeError,
                    std::format("else if condition must be boolean, got `{}`",
                        magic_enum::enum_name(elif->cond->resolvedType)),
                    "ensure condition is boolean",
                    makeSourceLocation(elif->cond)));
            }
            auto consequentRes = analyzeStmt(elif->consequent);
            if (!consequentRes)
            {
                return consequentRes;
            }
        }

        if (stmt->alternate)
        {
            auto alternateRes = analyzeStmt(stmt->alternate);
            if (!alternateRes)
            {
                return alternateRes;
            }
        }
        return {};
    }

    Result<void, Error> Analyzer::analyzeWhileStmt(WhileStmt *stmt)
    {
        auto condRes = analyzeExpr(stmt->cond);
        if (!condRes)
        {
            return condRes;
        }

        if (stmt->cond->resolvedType != TypeTag::Any && stmt->cond->resolvedType != TypeTag::Bool)
        {
            return std::unexpected(Error(ErrorType::TypeError,
                std::format("while condition must be boolean, got `{}`",
                    magic_enum::enum_name(stmt->cond->resolvedType)),
                "ensure condition is boolean",
                makeSourceLocation(stmt->cond)));
        }

        auto bodyRes = analyzeStmt(stmt->body);
        if (!bodyRes)
        {
            return bodyRes;
        }
        return {};
    }

    Result<void, Error> Analyzer::analyzeIdentiExpr(IdentiExpr *expr)
    {
        auto sym = env.Resolve(expr->name);
        if (sym == std::nullopt)
        {
            return std::unexpected(Error(ErrorType::UseUndeclaredIdentifier,
                std::format("`{}` has not been defined", expr->name),
                "none",
                makeSourceLocation(expr)));
        }
        // TODO: 引入 Module 跨文件 import，检查 isPublic
        expr->localId = sym->localId;

        expr->resolvedType  = sym->type;
        expr->resolvedDepth = sym->depth;
        expr->isGlobal = (sym->depth == 0);

        return {};
    }

    Result<void, Error> Analyzer::analyzeInfixExpr(InfixExpr *expr)
    {
        auto resL = analyzeExpr(expr->left);
        if (!resL)
            return std::unexpected(resL.error());

        auto resR = analyzeExpr(expr->right);
        if (!resR)
            return std::unexpected(resR.error());

        TypeTag lType = expr->left->resolvedType;
        TypeTag rType = expr->right->resolvedType;

        switch (expr->op)
        {
            // 算术族 (+, -, *, /, **)
            case BinaryOperator::Add:
                if (lType == TypeTag::String && rType == TypeTag::String)
                {
                    expr->resolvedType = TypeTag::String;
                    break;
                }
                [[fallthrough]];
            case BinaryOperator::Subtract:
            case BinaryOperator::Multiply:
            case BinaryOperator::Divide:
            case BinaryOperator::Power:
                if (lType == TypeTag::Int && rType == TypeTag::Int)
                {
                    expr->resolvedType = TypeTag::Int;
                }
                else if ((lType == TypeTag::Int || lType == TypeTag::Double)
                         && (rType == TypeTag::Int || rType == TypeTag::Double))
                {
                    expr->resolvedType = TypeTag::Double;
                }
                else if (lType == TypeTag::Any || rType == TypeTag::Any)
                {
                    expr->resolvedType = TypeTag::Any;
                }
                else
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "invalid operands for arithmetic operation",
                        "ensure both sides are numbers (Int or Double)",
                        makeSourceLocation(expr->right)));
                }
                break;

            // 整数特化族 (%, &, |, ^, <<, >>)
            case BinaryOperator::Modulo:
            case BinaryOperator::BitAnd:
            case BinaryOperator::BitOr:
            case BinaryOperator::BitXor:
            case BinaryOperator::ShiftLeft:
            case BinaryOperator::ShiftRight:
                if (lType == TypeTag::Int && rType == TypeTag::Int)
                {
                    expr->resolvedType = TypeTag::Int;
                }
                else if (lType == TypeTag::Any || rType == TypeTag::Any)
                {
                    expr->resolvedType = TypeTag::Any;
                }
                else
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "bitwise and modulo operations require Int operands",
                        "cast operands to Int before operation",
                        makeSourceLocation(expr->right)));
                }
                break;

            // 比较族 (==, !=, <, >, <=, >=)

            case BinaryOperator::Equal:
            case BinaryOperator::NotEqual:
            case BinaryOperator::Less:
            case BinaryOperator::Greater:
            case BinaryOperator::LessEqual:
            case BinaryOperator::GreaterEqual:
            case BinaryOperator::Is:
                if (lType != TypeTag::Any && rType != TypeTag::Any
                    && lType != rType) // lType == rType放行
                {
                    if (!((lType == TypeTag::Int && rType == TypeTag::Double)
                            || (lType == TypeTag::Double && rType == TypeTag::Int)))
                    {
                        return std::unexpected(Error(ErrorType::TypeError,
                            "cannot compare different types",
                            "ensure both sides of the comparison are of the same type",
                            makeSourceLocation(expr)));
                    }
                }

                // TODO: 支持Struct后进行检查，右操作数是 Struct才合理
                // 如 1.2 is Int --> false
                // 1 is Int --> true

                expr->resolvedType = TypeTag::Bool;
                break;

            // 逻辑族 (&&, ||)
            case BinaryOperator::LogicalAnd:
            case BinaryOperator::LogicalOr:
                if (lType == TypeTag::Bool && rType == TypeTag::Bool)
                {
                    expr->resolvedType = TypeTag::Bool;
                }
                else if (lType == TypeTag::Any || rType == TypeTag::Any)
                {
                    expr->resolvedType = TypeTag::Bool;
                }
                else
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "logical operators require Bool operands",
                        "use boolean expressions",
                        makeSourceLocation(expr)));
                }
                break;

            // 纯赋值与复合赋值族 (=, +=, -=, ...)
            case BinaryOperator::Assign:
            case BinaryOperator::AddAssign:
            case BinaryOperator::SubAssign:
            case BinaryOperator::MultiplyAssign:
            case BinaryOperator::DivideAssign:
            case BinaryOperator::ModuloAssign:
            case BinaryOperator::BitXorAssign:
                // 左侧必须是合法的 L-Value
                if (!isValidLvalue(expr->left))
                {
                    return std::unexpected(Error(ErrorType::NotAnLvalue,
                        "invalid assignment target",
                        "left side must be a variable, property, or indexable target",
                        makeSourceLocation(expr->left) // 错误精准定位到左侧节点
                        ));
                }

                // 类型匹配拦截 (纯赋值)
                if (expr->op == BinaryOperator::Assign)
                {
                    if (lType != TypeTag::Any && rType != TypeTag::Any && lType != rType)
                    {
                        if (!(lType == TypeTag::Double && rType == TypeTag::Int))
                        { // 允许 Int 赋给 Double
                            return std::unexpected(Error(ErrorType::TypeError,
                                "cannot assign value to variable of different type",
                                "ensure the assigned value matches the declared type",
                                makeSourceLocation(expr->right)));
                        }
                    }
                }
                expr->resolvedType = lType;
                break;

            // 成员访问 (.)
            case BinaryOperator::MemberAccess:
                if (lType != TypeTag::Struct && lType != TypeTag::Any)
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "member access requires a Struct object",
                        "check if the left side evaluates to an object",
                        makeSourceLocation(expr->left)));
                }
                if (expr->right->type != AstType::IdentiExpr)
                {
                    return std::unexpected(Error(
                        ErrorType::SyntaxError,
                        std::format("expect field name after member access '.', got {}", expr->right->toString()),
                        "none",
                        makeSourceLocation(expr->right)
                    ));
                }
                expr->resolvedType = TypeTag::Any;
                break;

            default:
                return std::unexpected(Error(ErrorType::TypeError,
                    "unknown binary operator in static analysis",
                    "this is likely an internal compiler error",
                    makeSourceLocation(expr)));
        }
        return {};
    }

    Result<void, Error> Analyzer::analyzeStmt(Stmt *stmt)
    {
        if (!stmt)
            return {};

        switch (stmt->type)
        {
            case AstType::VarDecl: return analyzeVarDecl(static_cast<VarDecl *>(stmt));

            case AstType::ExprStmt: {
                auto *exprStmt = static_cast<ExprStmt *>(stmt);
                return analyzeExpr(exprStmt->expr); // 表达式语句只需要推导内部表达式即可
            }

            case AstType::BlockStmt: {
                auto *block = static_cast<BlockStmt *>(stmt);
                env.EnterScope(); // 进入新大括号，作用域深度 +1
                for (auto *s : block->nodes)
                {
                    auto res = analyzeStmt(s);
                    if (!res)
                        return std::unexpected(res.error());
                }
                env.LeaveScope(); // 离开大括号，自动销毁局部类型记录
                return {};
            }

            case AstType::IfStmt: {
                return analyzeIfStmt(static_cast<IfStmt *>(stmt));
            }

            case AstType::WhileStmt: {
                return analyzeWhileStmt(static_cast<WhileStmt *>(stmt));
            }

                // TODO: 其他语句分析

                // default:
                //     return std::unexpected(Error(ErrorType::TypeError,
                //         "unsupported statement type in analyzer",
                //         "internal compiler error",
                //         makeSourceLocation(stmt)));
        }
        return {};
    }

    Result<void, Error> Analyzer::analyzeExpr(Expr *expr)
    {
        if (!expr)
            return {};

        switch (expr->type)
        {
            case AstType::LiteralExpr: {
                auto *lit = static_cast<LiteralExpr *>(expr);
                switch (lit->token.type)
                {
                    case TokenType::LiteralTrue:
                    case TokenType::LiteralFalse: lit->resolvedType = TypeTag::Bool; break;

                    case TokenType::LiteralNull: lit->resolvedType = TypeTag::Null; break;

                    case TokenType::LiteralNumber: {
                        const String &lexeme = manager.GetSub(lit->token.index, lit->token.length);
                        if (lexeme.contains(U'.') || lexeme.contains(U'e'))
                        {
                            lit->resolvedType = TypeTag::Double;
                        }
                        else
                        {
                            lit->resolvedType = TypeTag::Int;
                        }
                        break;
                    }

                    case TokenType::LiteralString: {
                        lit->resolvedType = TypeTag::String;
                        break;
                    }

                    default: {
                        lit->resolvedType = TypeTag::Any;
                        break;
                    }
                }
                return {};
            }

            case AstType::IdentiExpr: return analyzeIdentiExpr(static_cast<IdentiExpr *>(expr));

            case AstType::InfixExpr:
                return analyzeInfixExpr(static_cast<InfixExpr *>(expr));

                // TODO: PrefixExpr (前缀), CallExpr (函数调用), MemberExpr (属性访问)

            default:
                // 对于还没实现的表达式，默认降级为 Any 防止崩溃
                expr->resolvedType = TypeTag::Any;
                return {};
        }
    }

    Result<void, Error> Analyzer::Analyze(Program *program)
    {
        for (auto *stmt : program->nodes)
        {
            auto res = analyzeStmt(stmt);
            if (!res)
                return std::unexpected(res.error()); // 遇到任何错误，立刻中断并向上传递
        }
        return {};
    }

}; // namespace Fig