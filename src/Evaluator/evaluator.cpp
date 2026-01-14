#include <Ast/Statements/ErrorFlow.hpp>
#include <Value/VariableSlot.hpp>
#include <Value/value.hpp>
#include <Ast/AccessModifier.hpp>
#include <Ast/Statements/ImplementSt.hpp>
#include <Ast/Statements/InterfaceDefSt.hpp>
#include <Ast/astBase.hpp>
#include <Context/context_forward.hpp>
#include <Error/error.hpp>
#include <Value/Type.hpp>
#include <Value/interface.hpp>
#include <Value/structInstance.hpp>
#include <Error/errorLog.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>
#include <Module/builtins.hpp>
#include <Context/context.hpp>
#include <Utils/utils.hpp>
#include <Parser/parser.hpp>
#include <Core/executablePath.hpp>

#include <filesystem>
#include <fstream>
#include <memory>

namespace Fig
{

    bool Evaluator::isInterfaceSignatureMatch(const Ast::ImplementMethod &implMethod,
                                              const Ast::InterfaceMethod &ifMethod)
    {
        return implMethod.name == ifMethod.name && implMethod.paras == ifMethod.paras;
    }

    LvObject Evaluator::evalVarExpr(Ast::VarExpr var, ContextPtr ctx)
    {
        const FString &name = var->name;
        if (!ctx->contains(name)) { throw EvaluatorError(u8"UndeclaredIdentifierError", name, var); }
        return LvObject(ctx->get(name), ctx);
    }
    LvObject Evaluator::evalMemberExpr(Ast::MemberExpr me, ContextPtr ctx)
    {
        LvObject base = evalLv(me->base, ctx);
        RvObject baseVal = base.get();
        const FString &member = me->member;
        if (baseVal->getTypeInfo() == ValueType::Module)
        {
            const Module &mod = baseVal->as<Module>();
            if (mod.ctx->contains(member) && mod.ctx->isVariablePublic(member))
            {
                return LvObject(mod.ctx->get(member), ctx);
            }
            else
            {
                throw EvaluatorError(u8"VariableNotFoundError",
                                     std::format("`{}` has not variable '{}', check if it is public",
                                                 baseVal->toString().toBasicString(),
                                                 member.toBasicString()),
                                     me->base);
            }
        }
        if (baseVal->hasMemberFunction(member))
        {
            return LvObject(std::make_shared<VariableSlot>(
                                member,
                                std::make_shared<Object>(Function(baseVal->getMemberFunction(member),
                                                                  baseVal->getMemberFunctionParaCount(member))),
                                ValueType::Function,
                                AccessModifier::PublicConst),
                            ctx); // fake l-value
        }

        if (ctx->hasMethodImplemented(baseVal->getTypeInfo(), member))
        {
            // builtin type implementation!
            // e.g. impl xxx for Int

            auto &fn = ctx->getImplementedMethod(baseVal->getTypeInfo(), member);
            Function boundFn(fn.paras,
                             fn.retType,
                             fn.body,
                             ctx // current context
            );
            return LvObject(
                std::make_shared<VariableSlot>(
                    member, std::make_shared<Object>(boundFn), ValueType::Function, AccessModifier::PublicConst),
                ctx);
        }

        if (baseVal->getTypeInfo() != ValueType::StructInstance) // and not member function found
        {
            throw EvaluatorError(
                u8"NoAttributeError",
                std::format("`{}` has not attribute '{}'", baseVal->toString().toBasicString(), member.toBasicString()),
                me->base);
        }
        const StructInstance &si = baseVal->as<StructInstance>();
        if (ctx->hasMethodImplemented(si.parentType, member))
        {
            auto &fn = ctx->getImplementedMethod(si.parentType, member);
            Function boundFn(fn.paras,
                             fn.retType,
                             fn.body,
                             si.localContext // create a new function and set closure context
                                             // to struct instance context
            );
            return LvObject(
                std::make_shared<VariableSlot>(
                    member, std::make_shared<Object>(boundFn), ValueType::Function, AccessModifier::PublicConst),
                ctx);
        }
        else if (si.localContext->containsInThisScope(member) && si.localContext->isVariablePublic(member))
        {
            return LvObject(si.localContext->get(member), ctx);
        }
        else if (ctx->hasDefaultImplementedMethod(si.parentType, member))
        {
            return LvObject(std::make_shared<VariableSlot>(
                                member,
                                std::make_shared<Object>(ctx->getDefaultImplementedMethod(si.parentType, member)),
                                ValueType::Function,
                                AccessModifier::PublicConst),
                            ctx);
        }
        else
        {
            throw EvaluatorError(u8"NoAttributeError",
                                 std::format("`{}` has not attribute '{}' and no interfaces have been implemented it",
                                             baseVal->toString().toBasicString(),
                                             member.toBasicString()),
                                 me->base);
        }
    }
    LvObject Evaluator::evalIndexExpr(Ast::IndexExpr ie, ContextPtr ctx)
    {
        LvObject base = evalLv(ie->base, ctx);
        RvObject index = eval(ie->index, ctx);

        const TypeInfo &type = base.get()->getTypeInfo();

        if (type == ValueType::List)
        {
            if (index->getTypeInfo() != ValueType::Int)
            {
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Type `List` indices must be `Int`, got '{}'", prettyType(index).toBasicString()),
                    ie->index);
            }
            List &list = base.get()->as<List>();
            ValueType::IntClass indexVal = index->as<ValueType::IntClass>();
            if (indexVal >= list.size())
            {
                throw EvaluatorError(
                    u8"IndexOutOfRangeError",
                    std::format("Index {} out of list `{}` range", indexVal, base.get()->toString().toBasicString()),
                    ie->index);
            }
            return LvObject(base.get(), indexVal, LvObject::Kind::ListElement, ctx);
        }
        else if (type == ValueType::Map) { return LvObject(base.get(), index, LvObject::Kind::MapElement, ctx); }
        else if (type == ValueType::String)
        {
            if (index->getTypeInfo() != ValueType::Int)
            {
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Type `String` indices must be `Int`, got '{}'", prettyType(index).toBasicString()),
                    ie->index);
            }
            FString &string = base.get()->as<ValueType::StringClass>();
            ValueType::IntClass indexVal = index->as<ValueType::IntClass>();
            if (indexVal >= string.length())
            {
                throw EvaluatorError(
                    u8"IndexOutOfRangeError",
                    std::format("Index {} out of string `{}` range", indexVal, base.get()->toString().toBasicString()),
                    ie->index);
            }
            return LvObject(base.get(), indexVal, LvObject::Kind::StringElement, ctx);
        }
        else
        {
            throw EvaluatorError(
                u8"NoSubscriptableError",
                std::format("`{}` object is not subscriptable", base.declaredType().toString().toBasicString()),
                ie->base);
        }
    }
    LvObject Evaluator::evalLv(Ast::Expression exp, ContextPtr ctx)
    {
        using Ast::Operator;
        using Ast::AstType;
        switch (exp->getType())
        {
            case AstType::VarExpr: {
                Ast::VarExpr var = std::dynamic_pointer_cast<Ast::VarExprAst>(exp);
                assert(var != nullptr);
                return evalVarExpr(var, ctx);
            }
            case AstType::MemberExpr: {
                Ast::MemberExpr me = std::dynamic_pointer_cast<Ast::MemberExprAst>(exp);
                assert(me != nullptr);
                return evalMemberExpr(me, ctx);
            }
            case AstType::IndexExpr: {
                Ast::IndexExpr ie = std::dynamic_pointer_cast<Ast::IndexExprAst>(exp);
                assert(ie != nullptr);
                return evalIndexExpr(ie, ctx);
            }
            default: {
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Expression '{}' doesn't refer to a lvalue", exp->typeName().toBasicString()),
                    exp);
            }
        }
    }

    RvObject Evaluator::evalBinary(Ast::BinaryExpr bin, ContextPtr ctx)
    {
        using Ast::Operator;
        Operator op = bin->op;
        Ast::Expression lexp = bin->lexp, rexp = bin->rexp;
        switch (op)
        {
            case Operator::Add: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs + *rhs);
            }
            case Operator::Subtract: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs - *rhs);
            };
            case Operator::Multiply: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>((*lhs) * (*rhs));
            };
            case Operator::Divide: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs / *rhs);
            };
            case Operator::Modulo: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs % *rhs);
            };
            case Operator::Power: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(power(*lhs, *rhs));
            }
            case Operator::And: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs && *rhs);
            };
            case Operator::Or: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs || *rhs);
            };
            case Operator::Equal: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs == *rhs);
            }
            case Operator::NotEqual: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs != *rhs);
            }
            case Operator::Less: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs < *rhs);
            }
            case Operator::LessEqual: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs <= *rhs);
            }
            case Operator::Greater: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs > *rhs);
            }
            case Operator::GreaterEqual: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(*lhs >= *rhs);
            }
            case Operator::Is: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                if (lhs->is<StructInstance>() && rhs->is<StructType>())
                {
                    const StructInstance &si = lhs->as<StructInstance>();
                    const StructType &st = rhs->as<StructType>();
                    return std::make_shared<Object>(si.parentType == st.type);
                }
                if (lhs->is<StructInstance>() && rhs->is<InterfaceType>())
                {
                    const StructInstance &si = lhs->as<StructInstance>();
                    const InterfaceType &it = rhs->as<InterfaceType>();
                    return std::make_shared<Object>(implements(si.parentType, it.type, ctx));
                }
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Operator `is` requires an struct instance on left-hand side, got '{}'",
                                lhs->getTypeInfo().toString().toBasicString()),
                    bin->lexp);
            }

            case Operator::BitAnd: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(bit_and(*lhs, *rhs));
            }
            case Operator::BitOr: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(bit_or(*lhs, *rhs));
            }
            case Operator::BitXor: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(bit_xor(*lhs, *rhs));
            }
            case Operator::ShiftLeft: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(shift_left(*lhs, *rhs));
            }
            case Operator::ShiftRight: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return std::make_shared<Object>(shift_right(*lhs, *rhs));
            }

            case Operator::Assign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(rhs);
                return rhs;
            }
            case Operator::PlusAssign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(std::make_shared<Object>(*(lv.get()) + *rhs));
                return rhs;
            }
            case Operator::MinusAssign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(std::make_shared<Object>(*(lv.get()) - *rhs));
                return rhs;
            }
            case Operator::AsteriskAssign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(std::make_shared<Object>(*(lv.get()) * (*rhs)));
                return rhs;
            }
            case Operator::SlashAssign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(std::make_shared<Object>(*(lv.get()) / *rhs));
                return rhs;
            }
            case Operator::PercentAssign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(std::make_shared<Object>(*(lv.get()) / *rhs));
                return rhs;
            }
            // case Operator::CaretAssign: {
            //     LvObject lv = evalLv(lexp, ctx);
            //     ObjectPtr rhs = eval(rexp, ctx);
            //     lv.set(std::make_shared<Object>(
            //         *(lv.get()) ^ *rhs));
            // }
            default:
                throw EvaluatorError(u8"UnsupportedOp",
                                     std::format("Unsupport operator '{}' for binary", magic_enum::enum_name(op)),
                                     bin);
        }
    }
    RvObject Evaluator::evalUnary(Ast::UnaryExpr un, ContextPtr ctx)
    {
        using Ast::Operator;
        Operator op = un->op;
        Ast::Expression exp = un->exp;
        ObjectPtr value = eval(exp, ctx);
        switch (op)
        {
            case Operator::Not: {
                return std::make_shared<Object>(!(*value));
            }
            case Operator::Subtract: {
                return std::make_shared<Object>(-(*value));
            }
            case Operator::BitNot: {
                return std::make_shared<Object>(bit_not((*value)));
            }

            default: {
                throw EvaluatorError(u8"UnsupportedOpError",
                                     std::format("Unsupported op '{}' for unary expression", magic_enum::enum_name(op)),
                                     un);
            }
        }
    }
    RvObject Evaluator::evalTernary(Ast::TernaryExpr te, ContextPtr ctx)
    {
        RvObject condVal = eval(te->condition, ctx);
        if (condVal->getTypeInfo() != ValueType::Bool)
        {
            throw EvaluatorError(
                u8"TypeError",
                std::format("Condition must be boolean, got '{}'", prettyType(condVal).toBasicString()),
                te->condition);
        }
        ValueType::BoolClass cond = condVal->as<ValueType::BoolClass>();
        return (cond ? eval(te->valueT, ctx) : eval(te->valueF, ctx));
    }

    RvObject Evaluator::evalFunctionCall(const Function &fn,
                                         const Ast::FunctionArguments &fnArgs,
                                         const FString &fnName,
                                         ContextPtr ctx)
    {
        const Function &fnStruct = fn;
        Ast::FunctionCallArgs evaluatedArgs;
        if (fnStruct.isBuiltin)
        {
            for (const auto &argExpr : fnArgs.argv) { evaluatedArgs.argv.push_back(eval(argExpr, ctx)); }
            if (fnStruct.builtinParamCount != -1 && fnStruct.builtinParamCount != evaluatedArgs.getLength())
            {
                throw EvaluatorError(u8"BuiltinArgumentMismatchError",
                                     std::format("Builtin function '{}' expects {} arguments, but {} were provided",
                                                 fnName.toBasicString(),
                                                 fnStruct.builtinParamCount,
                                                 evaluatedArgs.getLength()),
                                     fnArgs.argv.back());
            }
            return fnStruct.builtin(evaluatedArgs.argv);
        }

        // check argument, all types of parameters
        Ast::FunctionParameters fnParas = fnStruct.paras;

        // create new context for function call
        auto newContext = std::make_shared<Context>(FString(std::format("<Function {}()>", fnName.toBasicString())),
                                                    fnStruct.closureContext);

        if (fnParas.variadic)
            goto VariadicFilling;
        else
            goto NormalFilling;

    NormalFilling: {
        if (fnArgs.getLength() < fnParas.posParas.size() || fnArgs.getLength() > fnParas.size())
        {
            throw RuntimeError(FString(std::format("Function '{}' expects {} to {} arguments, but {} were provided",
                                                   fnName.toBasicString(),
                                                   fnParas.posParas.size(),
                                                   fnParas.size(),
                                                   fnArgs.getLength())));
        }

        // positional parameters type check
        size_t i;
        for (i = 0; i < fnParas.posParas.size(); i++)
        {
            TypeInfo expectedType(fnParas.posParas[i].second); // look up type info, if exists a type with the
                                                               // name, use it, else throw
            ObjectPtr argVal = eval(fnArgs.argv[i], ctx);
            TypeInfo actualType = argVal->getTypeInfo();
            if (!isTypeMatch(expectedType, argVal, ctx))
            {
                throw EvaluatorError(u8"ArgumentTypeMismatchError",
                                     std::format("In function '{}', argument '{}' expects type '{}', but got type '{}'",
                                                 fnName.toBasicString(),
                                                 fnParas.posParas[i].first.toBasicString(),
                                                 expectedType.toString().toBasicString(),
                                                 actualType.toString().toBasicString()),
                                     fnArgs.argv[i]);
            }
            evaluatedArgs.argv.push_back(argVal);
        }
        // default parameters type check
        for (; i < fnArgs.getLength(); i++)
        {
            size_t defParamIndex = i - fnParas.posParas.size();
            TypeInfo expectedType(fnParas.defParas[defParamIndex].second.first);

            ObjectPtr defaultVal = eval(fnParas.defParas[defParamIndex].second.second, ctx);
            if (!isTypeMatch(expectedType, defaultVal, ctx))
            {
                throw EvaluatorError(
                    u8"DefaultParameterTypeError",
                    std::format(
                        "In function '{}', default parameter '{}' has type '{}', which does not match the expected type '{}'",
                        fnName.toBasicString(),
                        fnParas.defParas[defParamIndex].first.toBasicString(),
                        prettyType(defaultVal).toBasicString(),
                        expectedType.toString().toBasicString()),
                    fnArgs.argv[i]);
            }

            ObjectPtr argVal = eval(fnArgs.argv[i], ctx);
            TypeInfo actualType = argVal->getTypeInfo();
            if (!isTypeMatch(expectedType, argVal, ctx))
            {
                throw EvaluatorError(u8"ArgumentTypeMismatchError",
                                     std::format("In function '{}', argument '{}' expects type '{}', but got type '{}'",
                                                 fnName.toBasicString(),
                                                 fnParas.defParas[defParamIndex].first.toBasicString(),
                                                 expectedType.toString().toBasicString(),
                                                 actualType.toString().toBasicString()),
                                     fnArgs.argv[i]);
            }
            evaluatedArgs.argv.push_back(argVal);
        }
        // default parameters filling
        for (; i < fnParas.size(); i++)
        {
            size_t defParamIndex = i - fnParas.posParas.size();
            ObjectPtr defaultVal = eval(fnParas.defParas[defParamIndex].second.second, ctx);
            evaluatedArgs.argv.push_back(defaultVal);
        }

        // define parameters in new context
        for (size_t j = 0; j < fnParas.size(); j++)
        {
            FString paramName;
            TypeInfo paramType;
            if (j < fnParas.posParas.size())
            {
                paramName = fnParas.posParas[j].first;
                paramType = TypeInfo(fnParas.posParas[j].second);
            }
            else
            {
                size_t defParamIndex = j - fnParas.posParas.size();
                paramName = fnParas.defParas[defParamIndex].first;
                paramType = TypeInfo(fnParas.defParas[defParamIndex].second.first);
            }
            AccessModifier argAm = AccessModifier::Const;
            newContext->def(paramName, paramType, argAm, evaluatedArgs.argv[j]);
        }
        goto ExecuteBody;
    }

    VariadicFilling: {
        List list;
        for (auto &exp : fnArgs.argv)
        {
            list.push_back(eval(exp, ctx)); // eval arguments in current scope
        }
        newContext->def(fnParas.variadicPara, ValueType::List, AccessModifier::Const, std::make_shared<Object>(list));
        goto ExecuteBody;
    }

    ExecuteBody: {
        // execute function body
        ObjectPtr retVal = Object::getNullInstance();
        for (const auto &stmt : fnStruct.body->stmts)
        {
            StatementResult sr = evalStatement(stmt, newContext);
            if (sr.isError())
            {
                throw EvaluatorError(u8"UncaughtExceptionError",
                                     std::format("Uncaught exception: {}", sr.result->toString().toBasicString()),
                                     stmt);
            }
            if (!sr.isNormal())
            {
                retVal = sr.result;
                break;
            }
        }
        if (!isTypeMatch(fnStruct.retType, retVal, ctx))
        {
            throw EvaluatorError(u8"ReturnTypeMismatchError",
                                 std::format("Function '{}' expects return type '{}', but got type '{}'",
                                             fnName.toBasicString(),
                                             fnStruct.retType.toString().toBasicString(),
                                             prettyType(retVal).toBasicString()),
                                 fnStruct.body);
        }
        return retVal;
    }
    }

    RvObject Evaluator::eval(Ast::Expression exp, ContextPtr ctx)
    {
        using Ast::AstType;
        AstType type = exp->getType();
        switch (type)
        {
            case AstType::ValueExpr: {
                auto val = std::dynamic_pointer_cast<Ast::ValueExprAst>(exp);
                assert(val != nullptr);
                return val->val;
            }
            case AstType::VarExpr: {
                auto varExpr = std::dynamic_pointer_cast<Ast::VarExprAst>(exp);
                assert(varExpr != nullptr);
                return evalVarExpr(varExpr, ctx).get(); // LvObject -> RvObject
            }
            case AstType::BinaryExpr: {
                auto bin = std::dynamic_pointer_cast<Ast::BinaryExprAst>(exp);
                assert(bin != nullptr);
                return evalBinary(bin, ctx);
            }
            case AstType::UnaryExpr: {
                auto un = std::dynamic_pointer_cast<Ast::UnaryExprAst>(exp);
                assert(un != nullptr);
                return evalUnary(un, ctx);
            }
            case AstType::TernaryExpr: {
                auto te = std::dynamic_pointer_cast<Ast::TernaryExprAst>(exp);
                assert(te != nullptr);
                return evalTernary(te, ctx);
            }
            case AstType::MemberExpr:
            case AstType::IndexExpr: return evalLv(exp, ctx).get();

            case AstType::FunctionCall: {
                auto fnCall = std::dynamic_pointer_cast<Ast::FunctionCallExpr>(exp);
                assert(fnCall != nullptr);

                Ast::Expression callee = fnCall->callee;
                ObjectPtr fnObj = eval(callee, ctx);
                if (fnObj->getTypeInfo() != ValueType::Function)
                {
                    throw EvaluatorError(u8"ObjectNotCallable",
                                         std::format("Object `{}` isn't callable", fnObj->toString().toBasicString()),
                                         callee);
                }
                const Function &fn = fnObj->as<Function>();
                size_t fnId = fn.id;
                // const auto &fnNameOpt = ctx->getFunctionName(fnId);
                // const FString &fnName = (fnNameOpt ? *fnNameOpt :
                // u8"<anonymous>");

                auto fnNameOpt = ctx->getFunctionName(fnId);
                if (!fnNameOpt && fn.closureContext) fnNameOpt = fn.closureContext->getFunctionName(fnId);

                const FString &fnName = (fnNameOpt ? *fnNameOpt : u8"<anonymous>");

                return evalFunctionCall(fn, fnCall->arg, fnName, ctx);
            }
            case AstType::FunctionLiteralExpr: {
                auto fnLiteral = std::dynamic_pointer_cast<Ast::FunctionLiteralExprAst>(exp);
                assert(fnLiteral != nullptr);

                Ast::BlockStatement body = nullptr;
                if (fnLiteral->isExprMode())
                {
                    Ast::Expression exprBody = fnLiteral->getExprBody();
                    assert(exprBody != nullptr);

                    const Ast::AstAddressInfo &aai = exprBody->getAAI();
                    Ast::Return st = std::make_shared<Ast::ReturnSt>(exprBody);
                    st->setAAI(aai);

                    body = std::make_shared<Ast::BlockStatementAst>();
                    body->stmts.push_back(st); // convert to Ast::Statement
                    body->setAAI(aai);
                }
                else
                {
                    body = fnLiteral->getBlockBody();
                    assert(body != nullptr);
                }
                Function fn(fnLiteral->paras, ValueType::Any, body, ctx
                            /*
                                pass the ctx(fnLiteral eval context) as closure context
                            */
                );
                return std::make_shared<Object>(std::move(fn));
            }
            case AstType::InitExpr: {
                auto initExpr = std::dynamic_pointer_cast<Ast::InitExprAst>(exp);
                LvObject structeLv = evalLv(initExpr->structe, ctx);
                ObjectPtr structTypeVal = structeLv.get();
                const FString &structName = structeLv.name();
                if (!structTypeVal->is<StructType>())
                {
                    throw EvaluatorError(u8"NotAStructTypeError",
                                         std::format("'{}' is not a structure type", structName.toBasicString()),
                                         initExpr);
                }
                const StructType &structT = structTypeVal->as<StructType>();

                if (structT.builtin)
                {
                    const TypeInfo &type = structT.type;
                    auto &args = initExpr->args;
                    size_t argSize = args.size();

                    if (argSize > 1)
                    {
                        throw EvaluatorError(
                            u8"StructInitArgumentMismatchError",
                            std::format("Builtin class `{}` expects 0 or 1 argument, but {} were provided",
                                        type.toString().toBasicString(),
                                        argSize),
                            initExpr);
                    }

                    // default value
                    if (argSize == 0)
                    {
                        if (type == ValueType::Any || type == ValueType::Null || type == ValueType::Function)
                        {
                            throw EvaluatorError(
                                u8"BuiltinNotConstructibleError",
                                std::format("Builtin type `{}` cannot be constructed", type.toString().toBasicString()),
                                initExpr);
                        }
                        return std::make_shared<Object>(Object::defaultValue(type));
                    }

                    ObjectPtr val = eval(args[0].second, ctx);

                    auto err = [&](const char *msg) {
                        throw EvaluatorError(
                            u8"BuiltinInitTypeMismatchError",
                            std::format("Builtin `{}` constructor {}", type.toString().toBasicString(), msg),
                            initExpr);
                    };

                    // ===================== Int =====================
                    if (type == ValueType::Int)
                    {
                        if (!val->is<ValueType::IntClass>()) err("expects Int");
                        return std::make_shared<Object>(val->as<ValueType::IntClass>());
                    }

                    // ===================== Double =====================
                    if (type == ValueType::Double)
                    {
                        if (!val->is<ValueType::DoubleClass>()) err("expects Double");
                        return std::make_shared<Object>(val->as<ValueType::DoubleClass>());
                    }

                    // ===================== Bool =====================
                    if (type == ValueType::Bool)
                    {
                        if (!val->is<ValueType::BoolClass>()) err("expects Bool");
                        return std::make_shared<Object>(val->as<ValueType::BoolClass>());
                    }

                    // ===================== String =====================
                    if (type == ValueType::String)
                    {
                        if (!val->is<ValueType::StringClass>()) err("expects String");
                        return std::make_shared<Object>(val->as<ValueType::StringClass>());
                    }

                    // ===================== Null =====================
                    if (type == ValueType::Null)
                    {
                        // Null basically ignores input but keep invariant strict:
                        if (!val->is<ValueType::NullClass>()) err("expects Null");
                        return Object::getNullInstance();
                    }

                    // ===================== List =====================
                    if (type == ValueType::List)
                    {
                        if (!val->is<List>()) err("expects List");

                        const auto &src = val->as<List>();
                        auto copied = std::make_shared<Object>(List{});

                        auto &dst = copied->as<List>();
                        dst.reserve(src.size());
                        for (auto &e : src) dst.push_back(e); // shallow element copy, but new container

                        return copied;
                    }

                    // ===================== Map =====================
                    if (type == ValueType::Map)
                    {
                        if (!val->is<Map>()) err("expects Map");

                        const auto &src = val->as<Map>();
                        auto copied = std::make_shared<Object>(Map{});

                        auto &dst = copied->as<Map>();
                        for (auto &[k, v] : src) dst.emplace(k, v);

                        return copied;
                    }

                    throw EvaluatorError(
                        u8"BuiltinNotConstructibleError",
                        std::format("Builtin type `{}` cannot be constructed", type.toString().toBasicString()),
                        initExpr);
                }

                ContextPtr defContext = structT.defContext; // definition context
                // check init args

                size_t minArgs = 0;
                size_t maxArgs = structT.fields.size();

                for (auto &f : structT.fields)
                {
                    if (f.defaultValue == nullptr) minArgs++;
                }

                size_t got = initExpr->args.size();
                if (got > maxArgs || got < minArgs)
                {
                    throw EvaluatorError(u8"StructInitArgumentMismatchError",
                                         std::format("Structure '{}' expects {} to {} fields, but {} were provided",
                                                     structName.toBasicString(),
                                                     minArgs,
                                                     maxArgs,
                                                     initExpr->args.size()),
                                         initExpr);
                }

                std::vector<std::pair<FString, ObjectPtr>> evaluatedArgs;
                for (const auto &[argName, argExpr] : initExpr->args)
                {
                    evaluatedArgs.push_back({argName, eval(argExpr, ctx)});
                }
                ContextPtr instanceCtx = std::make_shared<Context>(
                    FString(std::format("<StructInstance {}>", structName.toBasicString())), ctx);
                /*
                    3 ways of calling constructor
                    .1 Person {"Fig", 1, "IDK"};
                    .2 Person {name: "Fig", age: 1, sex: "IDK"}; // can be unordered
                    .3 Person {name, age, sex};
                */
                {
                    using enum Ast::InitExprAst::InitMode;
                    if (initExpr->initMode == Positional)
                    {
                        for (size_t i = 0; i < maxArgs; ++i)
                        {
                            const Field &field = structT.fields[i];
                            const FString &fieldName = field.name;
                            const TypeInfo &expectedType = field.type;
                            if (i >= evaluatedArgs.size())
                            {
                                // we've checked argument count before, so here
                                // must be a default value

                                // evaluate default value in definition context
                                ObjectPtr defaultVal = eval(field.defaultValue,
                                                            ctx); // it can't be null here

                                // type check
                                if (!isTypeMatch(expectedType, defaultVal, ctx))
                                {
                                    throw EvaluatorError(
                                        u8"StructFieldTypeMismatchError",
                                        std::format(
                                            "In structure '{}', field '{}' expects type '{}', but got type '{}'",
                                            structName.toBasicString(),
                                            fieldName.toBasicString(),
                                            expectedType.toString().toBasicString(),
                                            prettyType(defaultVal).toBasicString()),
                                        initExpr);
                                }

                                instanceCtx->def(fieldName, expectedType, field.am, defaultVal);
                                continue;
                            }

                            const ObjectPtr &argVal = evaluatedArgs[i].second;
                            if (!isTypeMatch(expectedType, argVal, ctx))
                            {
                                throw EvaluatorError(
                                    u8"StructFieldTypeMismatchError",
                                    std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'",
                                                structName.toBasicString(),
                                                fieldName.toBasicString(),
                                                expectedType.toString().toBasicString(),
                                                prettyType(argVal).toBasicString()),
                                    initExpr);
                            }
                            instanceCtx->def(fieldName, expectedType, field.am, argVal);
                        }
                    }
                    else
                    {
                        // named / shorthand init
                        for (size_t i = 0; i < maxArgs; ++i)
                        {
                            const Field &field = structT.fields[i];
                            const FString &fieldName = (field.name.empty() ? evaluatedArgs[i].first : field.name);
                            if (instanceCtx->containsInThisScope(fieldName))
                            {
                                throw EvaluatorError(u8"StructFieldRedeclarationError",
                                                     std::format("Field '{}' already initialized in structure '{}'",
                                                                 fieldName.toBasicString(),
                                                                 structName.toBasicString()),
                                                     initExpr);
                            }
                            if (i + 1 > got)
                            {
                                // use default value                  //
                                // evaluate default value in definition context
                                ObjectPtr defaultVal = eval(field.defaultValue,
                                                            defContext); // it can't be null here

                                // type check
                                const TypeInfo &expectedType = field.type;
                                if (!isTypeMatch(expectedType, defaultVal, ctx))
                                {
                                    throw EvaluatorError(
                                        u8"StructFieldTypeMismatchError",
                                        std::format(
                                            "In structure '{}', field '{}' expects type '{}', but got type '{}'",
                                            structName.toBasicString(),
                                            fieldName.toBasicString(),
                                            expectedType.toString().toBasicString(),
                                            prettyType(defaultVal).toBasicString()),
                                        initExpr);
                                }

                                instanceCtx->def(fieldName, field.type, field.am, defaultVal);
                                continue;
                            }
                            const ObjectPtr &argVal = evaluatedArgs[i].second;
                            if (!isTypeMatch(field.type, argVal, ctx))
                            {
                                throw EvaluatorError(
                                    u8"StructFieldTypeMismatchError",
                                    std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'",
                                                structName.toBasicString(),
                                                fieldName.toBasicString(),
                                                field.type.toString().toBasicString(),
                                                prettyType(argVal).toBasicString()),
                                    initExpr);
                            }
                            instanceCtx->def(fieldName, field.type, field.am, argVal);
                        }
                    }
                }
                instanceCtx->merge(*structT.defContext);
                for (auto &[id, fn] : instanceCtx->getFunctions())
                {
                    instanceCtx->_update(*instanceCtx->getFunctionName(id),
                                         std::make_shared<Object>(Function(fn.paras,
                                                                           fn.retType,
                                                                           fn.body,
                                                                           instanceCtx) // change its closureContext to
                                                                                        // struct instance's context
                                                                  ));
                }
                return std::make_shared<Object>(StructInstance(structT.type, instanceCtx));
            }

            case AstType::ListExpr: {
                auto lstExpr = std::dynamic_pointer_cast<Ast::ListExprAst>(exp);
                assert(lstExpr != nullptr);

                List list;
                for (auto &exp : lstExpr->val) { list.push_back(eval(exp, ctx)); }
                return std::make_shared<Object>(std::move(list));
            }

            case AstType::MapExpr: {
                auto mapExpr = std::dynamic_pointer_cast<Ast::MapExprAst>(exp);
                assert(mapExpr != nullptr);

                Map map;
                for (auto &[key, value] : mapExpr->val) { map[eval(key, ctx)] = eval(value, ctx); }
                return std::make_shared<Object>(std::move(map));
            }

            default: assert(false);
        }
        return Object::getNullInstance(); // ignore warning
    }
    StatementResult Evaluator::evalBlockStatement(Ast::BlockStatement block, ContextPtr ctx)
    {
        StatementResult sr = StatementResult::normal();
        for (const Ast::Statement &stmt : block->stmts)
        {
            sr = evalStatement(stmt, ctx);
            if (!sr.isNormal()) { return sr; }
        }
        return sr;
    }
    StatementResult Evaluator::evalStatement(Ast::Statement stmt, ContextPtr ctx)
    {
        using enum Ast::AstType;
        switch (stmt->getType())
        {
            case ImportSt: {
                auto i = std::dynamic_pointer_cast<Ast::ImportSt>(stmt);
                assert(i != nullptr);
                return evalImportSt(i, ctx);
            }
            case VarDefSt: {
                auto varDef = std::dynamic_pointer_cast<Ast::VarDefAst>(stmt);
                assert(varDef != nullptr);

                if (ctx->containsInThisScope(varDef->name))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Variable `{}` already declared in this scope", varDef->name.toBasicString()),
                        varDef);
                }

                RvObject value = nullptr;
                if (varDef->expr) { value = eval(varDef->expr, ctx); }
                TypeInfo declaredType; // default is Any
                const FString &declaredTypeName = varDef->typeName;
                if (declaredTypeName == Parser::varDefTypeFollowed) { declaredType = value->getTypeInfo(); }
                else if (!declaredTypeName.empty())
                {
                    declaredType = TypeInfo(declaredTypeName);
                    if (value != nullptr && !isTypeMatch(declaredType, value, ctx))
                    {
                        throw EvaluatorError(u8"TypeError",
                                             std::format("Variable `{}` expects init-value type `{}`, but got '{}'",
                                                         varDef->name.toBasicString(),
                                                         declaredTypeName.toBasicString(),
                                                         prettyType(value).toBasicString()),
                                             varDef->expr);
                    }
                    else if (value == nullptr)
                    {
                        value = std::make_shared<Object>(Object::defaultValue(declaredType));
                    } // else -> Ok
                } // else -> type is Any (default)
                AccessModifier am =
                    (varDef->isConst ? (varDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const) :
                                       (varDef->isPublic ? AccessModifier::Public : AccessModifier::Normal));
                ctx->def(varDef->name, declaredType, am, value);
                return StatementResult::normal();
            }

            case FunctionDefSt: {
                auto fnDef = std::dynamic_pointer_cast<Ast::FunctionDefSt>(stmt);
                assert(fnDef != nullptr);

                const FString &fnName = fnDef->name;
                if (ctx->containsInThisScope(fnName))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Function `{}` already declared in this scope", fnName.toBasicString()),
                        fnDef);
                }
                Function fn(fnDef->paras, TypeInfo(fnDef->retType), fnDef->body, ctx);
                ctx->def(fnName,
                         ValueType::Function,
                         (fnDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const),
                         std::make_shared<Object>(fn));
                return StatementResult::normal();
            }

            case StructSt: {
                auto stDef = std::dynamic_pointer_cast<Ast::StructDefSt>(stmt);
                assert(stDef != nullptr);

                if (ctx->containsInThisScope(stDef->name))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Structure '{}' already defined in this scope", stDef->name.toBasicString()),
                        stDef);
                }
                std::vector<Field> fields;
                std::vector<FString> _fieldNames;
                for (Ast::StructDefField field : stDef->fields)
                {
                    if (Utils::vectorContains(field.fieldName, _fieldNames))
                    {
                        throw EvaluatorError(u8"RedeclarationError",
                                             std::format("Field '{}' already defined in structure '{}'",
                                                         field.fieldName.toBasicString(),
                                                         stDef->name.toBasicString()),
                                             stDef);
                    }
                    fields.push_back(Field(field.am, field.fieldName, TypeInfo(field.tiName), field.defaultValueExpr));
                }
                ContextPtr defContext = std::make_shared<Context>(FString(std::format("<Struct {} at {}:{}>",
                                                                                      stDef->name.toBasicString(),
                                                                                      stDef->getAAI().line,
                                                                                      stDef->getAAI().column)),
                                                                  ctx);
                const Ast::BlockStatement &body = stDef->body;
                for (auto &st : body->stmts)
                {
                    if (st->getType() != Ast::AstType::FunctionDefSt)
                    {
                        throw EvaluatorError(u8"UnexpectedStatementInStructError",
                                             std::format("Unexpected statement `{}` in struct declaration",
                                                         st->toString().toBasicString()),
                                             st);
                    }
                    evalStatement(st, defContext); // function def st
                }

                AccessModifier am = (stDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const);
                TypeInfo type(stDef->name, true); // register type name
                ctx->def(stDef->name,
                         ValueType::StructType,
                         am,
                         std::make_shared<Object>(StructType(type, defContext, fields)));
                return StatementResult::normal();
            }

            case InterfaceDefSt: {
                auto ifd = std::dynamic_pointer_cast<Ast::InterfaceDefAst>(stmt);
                assert(ifd != nullptr);

                const FString &interfaceName = ifd->name;

                if (ctx->containsInThisScope(interfaceName))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Interface `{}` already declared in this scope", interfaceName.toBasicString()),
                        ifd);
                }
                TypeInfo type(interfaceName, true); // register interface
                ctx->def(interfaceName,
                         type,
                         (ifd->isPublic ? AccessModifier::PublicConst : AccessModifier::Const),
                         std::make_shared<Object>(InterfaceType(type, ifd->methods)));
                return StatementResult::normal();
            }

            case ImplementSt: {
                auto ip = std::dynamic_pointer_cast<Ast::ImplementAst>(stmt);
                assert(ip != nullptr);

                TypeInfo structType(ip->structName);
                TypeInfo interfaceType(ip->interfaceName);
                if (ctx->hasImplRegisted(structType, interfaceType))
                {
                    throw EvaluatorError(u8"DuplicateImplError",
                                         std::format("Duplicate implement `{}` for `{}`",
                                                     interfaceType.toString().toBasicString(),
                                                     structType.toString().toBasicString()),
                                         ip);
                }
                if (!ctx->contains(ip->interfaceName))
                {
                    throw EvaluatorError(u8"InterfaceNotFoundError",
                                         std::format("Interface '{}' not found", ip->interfaceName.toBasicString()),
                                         ip);
                }
                if (!ctx->contains(ip->structName))
                {
                    throw EvaluatorError(u8"StructNotFoundError",
                                         std::format("Struct '{}' not found", ip->structName.toBasicString()),
                                         ip);
                }
                auto interfaceSlot = ctx->get(ip->interfaceName);
                auto structSlot = ctx->get(ip->structName);

                LvObject interfaceLv(interfaceSlot, ctx);
                LvObject structLv(structSlot, ctx);

                ObjectPtr interfaceObj = interfaceLv.get();
                ObjectPtr structTypeObj = structLv.get();

                if (!interfaceObj->is<InterfaceType>())
                {
                    throw EvaluatorError(
                        u8"NotAInterfaceError",
                        std::format("Variable `{}` is not a interface", ip->interfaceName.toBasicString()),
                        ip);
                }
                if (!structTypeObj->is<StructType>())
                {
                    throw EvaluatorError(
                        u8"NotAStructType",
                        std::format("Variable `{}` is not a struct type", ip->structName.toBasicString()),
                        ip);
                }
                auto &implementMethods = ip->methods;
                InterfaceType &interface = interfaceObj->as<InterfaceType>();

                // ===== interface implementation validation =====
                ImplRecord record{interfaceType, structType, {}};

                std::unordered_map<FString, Ast::InterfaceMethod> ifaceMethods;
                for (auto &m : interface.methods)
                {
                    if (ifaceMethods.contains(m.name))
                    {
                        throw EvaluatorError(u8"InterfaceDuplicateMethodError",
                                             std::format("Interface '{}' has duplicate method '{}'",
                                                         interfaceType.toString().toBasicString(),
                                                         m.name.toBasicString()),
                                             ip);
                    }
                    ifaceMethods[m.name] = m;
                }

                std::unordered_set<FString> implemented;

                for (auto &implMethod : implementMethods)
                {
                    const FString &name = implMethod.name;

                    // ---- redundant impl ----
                    if (!ifaceMethods.contains(name))
                    {
                        throw EvaluatorError(u8"RedundantImplementationError",
                                             std::format("Struct '{}' implements extra method '{}' "
                                                         "which is not required by interface '{}'",
                                                         structType.toString().toBasicString(),
                                                         name.toBasicString(),
                                                         interfaceType.toString().toBasicString()),
                                             ip);
                    }

                    if (implemented.contains(name))
                    {
                        throw EvaluatorError(u8"DuplicateImplementMethodError",
                                             std::format("Duplicate implement method '{}'", name.toBasicString()),
                                             ip);
                    }

                    auto &ifMethod = ifaceMethods[name];

                    // ---- signature check ----
                    if (!isInterfaceSignatureMatch(implMethod, ifMethod))
                    {
                        throw EvaluatorError(u8"InterfaceSignatureMismatch",
                                             std::format("Interface method '{}({})' signature mismatch with "
                                                         "implementation '{}({})'",
                                                         ifMethod.name.toBasicString(),
                                                         ifMethod.paras.toString().toBasicString(),
                                                         implMethod.name.toBasicString(),
                                                         implMethod.paras.toString().toBasicString()),
                                             ip);
                    }

                    if (ctx->hasMethodImplemented(structType, name))
                    {
                        throw EvaluatorError(u8"DuplicateImplementMethodError",
                                             std::format("Method '{}' already implemented by another interface "
                                                         "for struct '{}'",
                                                         name.toBasicString(),
                                                         structType.toString().toBasicString()),
                                             ip);
                    }

                    implemented.insert(name);

                    record.implMethods[name] =
                        Function(implMethod.paras, TypeInfo(ifMethod.returnType), implMethod.body, ctx);
                }

                for (auto &m : interface.methods)
                {
                    if (implemented.contains(m.name)) continue;

                    if (m.hasDefaultBody()) continue;

                    throw EvaluatorError(u8"MissingImplementationError",
                                         std::format("Struct '{}' does not implement required interface method '{}' "
                                                     "and interface '{}' provides no default implementation",
                                                     structType.toString().toBasicString(),
                                                     m.name.toBasicString(),
                                                     interfaceType.toString().toBasicString()),
                                         ip);
                }

                ctx->setImplRecord(structType, interfaceType, record);
                return StatementResult::normal();
            }

            case IfSt: {
                auto ifSt = std::dynamic_pointer_cast<Ast::IfSt>(stmt);
                ObjectPtr condVal = eval(ifSt->condition, ctx);
                if (condVal->getTypeInfo() != ValueType::Bool)
                {
                    throw EvaluatorError(
                        u8"TypeError",
                        std::format("Condition must be boolean, but got '{}'", prettyType(condVal).toBasicString()),
                        ifSt->condition);
                }
                if (condVal->as<ValueType::BoolClass>()) { return evalBlockStatement(ifSt->body, ctx); }
                // else
                for (const auto &elif : ifSt->elifs)
                {
                    ObjectPtr elifCondVal = eval(elif->condition, ctx);
                    if (elifCondVal->getTypeInfo() != ValueType::Bool)
                    {
                        throw EvaluatorError(
                            u8"TypeError",
                            std::format("Condition must be boolean, but got '{}'", prettyType(condVal).toBasicString()),
                            ifSt->condition);
                    }
                    if (elifCondVal->as<ValueType::BoolClass>()) { return evalBlockStatement(elif->body, ctx); }
                }
                if (ifSt->els) { return evalBlockStatement(ifSt->els->body, ctx); }
                return StatementResult::normal();
            };
            case WhileSt: {
                auto whileSt = std::dynamic_pointer_cast<Ast::WhileSt>(stmt);
                while (true)
                {
                    ObjectPtr condVal = eval(whileSt->condition, ctx);
                    if (condVal->getTypeInfo() != ValueType::Bool)
                    {
                        throw EvaluatorError(
                            u8"TypeError",
                            std::format("Condition must be boolean, but got '{}'", prettyType(condVal).toBasicString()),
                            whileSt->condition);
                    }
                    if (!condVal->as<ValueType::BoolClass>()) { break; }
                    ContextPtr loopContext = std::make_shared<Context>(
                        FString(std::format("<While {}:{}>", whileSt->getAAI().line, whileSt->getAAI().column)),
                        ctx); // every loop has its own context
                    StatementResult sr = evalBlockStatement(whileSt->body, loopContext);
                    if (sr.shouldReturn()) { return sr; }
                    if (sr.shouldBreak()) { break; }
                    if (sr.shouldContinue()) { continue; }
                }
                return StatementResult::normal();
            };
            case ForSt: {
                auto forSt = std::dynamic_pointer_cast<Ast::ForSt>(stmt);
                ContextPtr loopContext = std::make_shared<Context>(
                    FString(std::format("<For {}:{}>", forSt->getAAI().line, forSt->getAAI().column)),
                    ctx); // for loop has its own context

                evalStatement(forSt->initSt,
                              loopContext); // ignore init statement result
                size_t iteration = 0;

                while (true) // use while loop to simulate for loop, cause we
                             // need to check condition type every iteration
                {
                    ObjectPtr condVal = eval(forSt->condition, loopContext);
                    if (condVal->getTypeInfo() != ValueType::Bool)
                    {
                        throw EvaluatorError(
                            u8"TypeError",
                            std::format("Condition must be boolean, but got '{}'", prettyType(condVal).toBasicString()),
                            forSt->condition);
                    }
                    if (!condVal->as<ValueType::BoolClass>()) { break; }
                    iteration++;
                    ContextPtr iterationContext = std::make_shared<Context>(
                        FString(std::format(
                            "<For {}:{}, Iteration {}>", forSt->getAAI().line, forSt->getAAI().column, iteration)),
                        loopContext); // every loop has its own context
                    StatementResult sr = evalBlockStatement(forSt->body, iterationContext);
                    if (sr.shouldReturn()) { return sr; }
                    if (sr.shouldBreak()) { break; }
                    if (sr.shouldContinue())
                    {
                        // continue to next iteration
                        continue;
                    }
                    evalStatement(forSt->incrementSt,
                                  loopContext); // ignore increment statement result
                }
                return StatementResult::normal();
            }

            case TrySt: {
                auto tryst = std::dynamic_pointer_cast<Ast::TrySt>(stmt);
                assert(tryst != nullptr);

                ContextPtr tryCtx = std::make_shared<Context>(
                    FString(std::format("<Try at {}:{}>", tryst->getAAI().line, tryst->getAAI().column)), ctx);
                StatementResult sr = StatementResult::normal();
                for (auto &stmt : tryst->body->stmts)
                {
                    sr = evalStatement(stmt, tryCtx); // eval in try context
                    if (sr.isError()) { break; }
                }
                bool catched = false;
                for (auto &cat : tryst->catches)
                {
                    const FString &errVarName = cat.errVarName;
                    TypeInfo errVarType = (cat.hasType ? TypeInfo(cat.errVarType) : ValueType::Any);
                    if (isTypeMatch(errVarType, sr.result, ctx))
                    {
                        ContextPtr catchCtx = std::make_shared<Context>(
                            FString(
                                std::format("<Catch at {}:{}>", cat.body->getAAI().line, cat.body->getAAI().column)),
                            ctx);
                        catchCtx->def(errVarName, errVarType, AccessModifier::Normal, sr.result);
                        sr = evalBlockStatement(cat.body, catchCtx);
                        catched = true;
                        break;
                    }
                }
                if (!catched)
                {
                    throw EvaluatorError(u8"UncaughtExceptionError",
                                         std::format("Uncaught exception: {}", sr.result->toString().toBasicString()),
                                         tryst);
                }
                if (tryst->finallyBlock) { sr = evalBlockStatement(tryst->finallyBlock, ctx); }
                return sr;
            }

            case ThrowSt: {
                auto ts = std::dynamic_pointer_cast<Ast::ThrowSt>(stmt);
                assert(ts != nullptr);

                ObjectPtr value = eval(ts->value, ctx);
                if (value->is<ValueType::NullClass>())
                {
                    throw EvaluatorError(u8"TypeError", u8"Why did you throw a null?", ts);
                }
                return StatementResult::errorFlow(value);
            }

            case ReturnSt: {
                auto returnSt = std::dynamic_pointer_cast<Ast::ReturnSt>(stmt);
                assert(returnSt != nullptr);

                ObjectPtr returnValue = Object::getNullInstance(); // default is null
                if (returnSt->retValue) returnValue = eval(returnSt->retValue, ctx);
                return StatementResult::returnFlow(returnValue);
            }

            case BreakSt: {
                if (!ctx->parent)
                {
                    throw EvaluatorError(u8"BreakOutsideLoopError", u8"`break` statement outside loop", stmt);
                }
                if (!ctx->isInLoopContext())
                {
                    throw EvaluatorError(u8"BreakOutsideLoopError", u8"`break` statement outside loop", stmt);
                }
                return StatementResult::breakFlow();
            }

            case ContinueSt: {
                if (!ctx->parent)
                {
                    throw EvaluatorError(u8"ContinueOutsideLoopError", u8"`continue` statement outside loop", stmt);
                }
                if (!ctx->isInLoopContext())
                {
                    throw EvaluatorError(u8"ContinueOutsideLoopError", u8"`continue` statement outside loop", stmt);
                }
                return StatementResult::continueFlow();
            }

            case ExpressionStmt: {
                auto exprStmt = std::dynamic_pointer_cast<Ast::ExpressionStmtAst>(stmt);
                assert(exprStmt != nullptr);

                return StatementResult::normal(eval(exprStmt->exp, ctx));
            }

            default:
                throw RuntimeError(
                    FString(std::format("Feature stmt {} unsupported yet", magic_enum::enum_name(stmt->getType()))));
        }
    }

    std::filesystem::path Evaluator::resolveModulePath(const std::vector<FString> &pathVec)
    {
        namespace fs = std::filesystem;

        static const std::vector<fs::path> defaultLibraryPath{"Library", "Library/fpm"};

        std::vector<fs::path> pathToFind(defaultLibraryPath);

        fs::path interpreterPath = getExecutablePath().parent_path();

        for (fs::path &p : pathToFind)
        {
            p = interpreterPath / p; // 相对路径 -> 绝对路径
        }

        pathToFind.insert(
            pathToFind.begin(),
            fs::path(this->sourcePath.toBasicString()).parent_path()); // first search module at the source file path

        fs::path path;

        /*
        Example:
            import comp.config;
        */

        const FString &modPathStrTop = pathVec.at(0);
        fs::path modPath;

        bool found = false;
        for (auto &parentFolder : pathToFind)
        {
            modPath = parentFolder / FString(modPathStrTop + u8".fig").toBasicString();
            if (fs::exists(modPath))
            {
                path = modPath;
                found = true;
                break;
            }
            else
            {
                modPath = parentFolder / modPathStrTop.toBasicString();
                if (fs::is_directory(modPath)) // comp is a directory
                {
                    modPath = modPath / FString(modPathStrTop + u8".fig").toBasicString();
                    /*
                        if module name is a directory, we require [module
                       name].fig at the directory
                    */
                    if (!fs::exists(modPath))
                    {
                        throw RuntimeError(FString(std::format("requires module file, {}\\{}",
                                                               modPathStrTop.toBasicString(),
                                                               FString(modPathStrTop + u8".fig").toBasicString())));
                    }
                    found = true;
                    path = modPath;
                    break;
                }
            }
        }

        if (!found)
            throw RuntimeError(FString(std::format("Could not find module `{}`", modPathStrTop.toBasicString())));

        bool found2 = false;

        for (size_t i = 1; i < pathVec.size(); ++i) // has next module
        {
            const FString &next = pathVec.at(i);
            modPath = modPath.parent_path(); // get the folder
            modPath = modPath / FString(next + u8".fig").toBasicString();
            if (fs::exists(modPath))
            {
                if (i != pathVec.size() - 1)
                    throw RuntimeError(FString(std::format(
                        "expects {} as parent directory and find next module, but got a file", next.toBasicString())));
                // it's the last module
                found2 = true;
                path = modPath;
                break;
            }
            // `next` is a folder
            modPath = modPath.parent_path() / next.toBasicString();
            if (!fs::exists(modPath))
                throw RuntimeError(FString(std::format("Could not find module `{}`", next.toBasicString())));
            if (i == pathVec.size() - 1)
            {
                // `next` is the last module
                modPath = modPath / FString(next + u8".fig").toBasicString();
                if (!fs::exists(modPath))
                {
                    throw RuntimeError(FString(std::format(
                        "expects {} as parent directory and find next module, but got a file", next.toBasicString())));
                }
                found2 = true;
                path = modPath;
            }
        }

        if (!found2 && !fs::exists(modPath))
            throw RuntimeError(FString(std::format("Could not find module `{}`", pathVec.end()->toBasicString())));

        return path;
    }

    ContextPtr Evaluator::loadModule(const std::filesystem::path &path)
    {
        std::ifstream file(path);
        assert(file.is_open());

        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        Lexer lexer((FString(source)));
        Parser parser(lexer);
        std::vector<Ast::AstBase> asts;

        std::vector<FString> sourceLines = Utils::splitSource(FString(source));

        asts = parser.parseAll();

        Evaluator evaluator;
        evaluator.SetSourcePath(FString(path.string()));

        ContextPtr modctx = std::make_shared<Context>(FString(std::format("<Module at {}>", path.string())), nullptr);

        evaluator.SetGlobalContext(modctx);
        evaluator.RegisterBuiltinsValue();
        evaluator.Run(asts);

        return evaluator.global;
    }

    StatementResult Evaluator::evalImportSt(Ast::Import i, ContextPtr ctx)
    {
        const std::vector<FString> &pathVec = i->path;

        const FString &modName = pathVec.at(pathVec.size() - 1); // pathVec at least has 1 element
        if (modName == u8"_builtins")
        {
            RegisterBuiltins();
            return StatementResult::normal();
        }
        auto path = resolveModulePath(pathVec);
        ContextPtr modCtx = loadModule(path);

        // std::cerr << modName.toBasicString() << '\n'; DEBUG

        if (ctx->containsInThisScope(modName))
        {
            throw EvaluatorError(
                u8"RedeclarationError", std::format("{} has already been declared.", modName.toBasicString()), i);
        }
        ctx->def(
            modName, ValueType::Module, AccessModifier::PublicConst, std::make_shared<Object>(Module(modName, modCtx)));
        return StatementResult::normal();
    }

    StatementResult Evaluator::Run(std::vector<Ast::AstBase> asts)
    {
        using Ast::AstType;
        StatementResult sr = StatementResult::normal();
        for (auto &ast : asts)
        {
            Ast::Expression exp;
            if ((exp = std::dynamic_pointer_cast<Ast::ExpressionAst>(ast)))
            {
                sr = StatementResult::normal(eval(exp, global));
            }
            else
            {
                // statement
                Ast::Statement stmt = std::dynamic_pointer_cast<Ast::StatementAst>(ast);
                assert(stmt != nullptr);
                sr = evalStatement(stmt, global);
                if (sr.isError())
                {
                    throw EvaluatorError(u8"UncaughtExceptionError",
                                         std::format("Uncaught exception: {}", sr.result->toString().toBasicString()),
                                         stmt);
                }
                if (!sr.isNormal()) { return sr; }
            }
        }

        return sr;
    }

    void Evaluator::printStackTrace()
    {
        if (global) global->printStackTrace();
    }
}; // namespace Fig