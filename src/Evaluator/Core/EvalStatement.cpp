#include <Evaluator/Core/ExprResult.hpp>
#include <Ast/AccessModifier.hpp>
#include <Ast/Expressions/FunctionCall.hpp>
#include <Ast/astBase.hpp>
#include <Ast/functionParameters.hpp>
#include <Core/fig_string.hpp>
#include <Evaluator/Core/StatementResult.hpp>
#include <Evaluator/Value/Type.hpp>
#include <Evaluator/Value/structType.hpp>
#include <Evaluator/Value/value.hpp>
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

#include <Utils/utils.hpp>
#include <unordered_map>

namespace Fig
{
    StatementResult Evaluator::evalStatement(Ast::Statement stmt, ContextPtr ctx)
    {
        using enum Ast::AstType;
        switch (stmt->getType())
        {
            case ImportSt: {
                auto i = std::static_pointer_cast<Ast::ImportSt>(stmt);
                return evalImportSt(i, ctx);
            }
            case VarDefSt: {
                auto varDef = std::static_pointer_cast<Ast::VarDefAst>(stmt);

                if (ctx->containsInThisScope(varDef->name))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Variable `{}` already declared in this scope", varDef->name.toBasicString()),
                        varDef);
                }

                RvObject value = nullptr;
                if (varDef->expr) { value = check_unwrap_stres(eval(varDef->expr, ctx)); }

                TypeInfo declaredType; // default is Any
                const Ast::Expression &declaredTypeExp = varDef->declaredType;

                if (varDef->followupType) { declaredType = actualType(value); }
                else if (declaredTypeExp)
                {
                    ObjectPtr declaredTypeValue = check_unwrap_stres(eval(declaredTypeExp, ctx));
                    declaredType = actualType(declaredTypeValue);

                    if (value != nullptr && !isTypeMatch(declaredType, value, ctx))
                    {
                        throw EvaluatorError(u8"TypeError",
                                             std::format("Variable `{}` expects init-value type `{}`, but got '{}'",
                                                         varDef->name.toBasicString(),
                                                         prettyType(declaredTypeValue).toBasicString(),
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
                auto fnDef = std::static_pointer_cast<Ast::FunctionDefSt>(stmt);

                const FString &fnName = fnDef->name;
                if (ctx->containsInThisScope(fnName))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Function `{}` already declared in this scope", fnName.toBasicString()),
                        fnDef);
                }
                TypeInfo returnType = ValueType::Any;
                if (fnDef->retType)
                {
                    ObjectPtr returnTypeValue = check_unwrap_stres(eval(fnDef->retType, ctx));
                    returnType = actualType(returnTypeValue);
                }

                Function fn(fnName, fnDef->paras, returnType, fnDef->body, ctx);
                ctx->def(fnName,
                         ValueType::Function,
                         (fnDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const),
                         std::make_shared<Object>(fn));
                return StatementResult::normal();
            }

            case StructSt: {
                auto stDef = std::static_pointer_cast<Ast::StructDefSt>(stmt);

                if (ctx->containsInThisScope(stDef->name))
                {
                    throw EvaluatorError(
                        u8"RedeclarationError",
                        std::format("Structure '{}' already defined in this scope", stDef->name.toBasicString()),
                        stDef);
                }

                TypeInfo type(stDef->name, true); // register type name
                ContextPtr defContext = std::make_shared<Context>(FString(std::format("<Struct {} at {}:{}>",
                                                                                      stDef->name.toBasicString(),
                                                                                      stDef->getAAI().line,
                                                                                      stDef->getAAI().column)),
                                                                  ctx);
                ObjectPtr structTypeObj = std::make_shared<Object>(StructType(type, defContext, {}));

                AccessModifier am = (stDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const);

                ctx->def(stDef->name, ValueType::StructType, am, structTypeObj); // predef
                defContext->def(stDef->name,
                                ValueType::StructType,
                                AccessModifier::Const,
                                structTypeObj); // predef to itself, always const

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
                    TypeInfo fieldType = ValueType::Any;
                    if (field.declaredType)
                    {
                        ObjectPtr declaredTypeValue = check_unwrap_stres(eval(field.declaredType, ctx));
                        fieldType = actualType(declaredTypeValue);
                    }

                    fields.push_back(Field(field.am, field.fieldName, fieldType, field.defaultValueExpr));
                }
                structTypeObj->as<StructType>().fields = fields;

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
                return StatementResult::normal();
            }

            case InterfaceDefSt: {
                auto ifd = std::static_pointer_cast<Ast::InterfaceDefAst>(stmt);

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
                auto ip = std::static_pointer_cast<Ast::ImplementAst>(stmt);

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

                if (ip->interfaceName == u8"Operation")
                {
                    // 运算符重载
                    /*
                    impl Operation for xxx
                    {
                        add(l, r) {...}
                    }
                    */
                    if (ValueType::isTypeBuiltin(structType))
                    {
                        throw EvaluatorError(u8"BadUserError",
                                             std::format("Don't overload built-in type operators plz! `{}`",
                                                         prettyType(structTypeObj).toBasicString()),
                                             ip);
                    }

                    using enum Ast::Operator;
                    static const std::unordered_map<FString, std::pair<Ast::Operator, size_t>> magic_name_to_op = {
                        // 算术
                        {u8"Add", {Ast::Operator::Add, 2}},
                        {u8"Sub", {Ast::Operator::Subtract, 2}},
                        {u8"Mul", {Ast::Operator::Multiply, 2}},
                        {u8"Div", {Ast::Operator::Divide, 2}},
                        {u8"Mod", {Ast::Operator::Modulo, 2}},
                        {u8"Pow", {Ast::Operator::Power, 2}},

                        // 逻辑（一元）
                        {u8"Neg", {Ast::Operator::Subtract, 1}}, // 一元负号
                        {u8"Not", {Ast::Operator::Not, 1}},

                        // 逻辑（二元）
                        {u8"And", {Ast::Operator::And, 2}},
                        {u8"Or", {Ast::Operator::Or, 2}},

                        // 比较
                        {u8"Equal", {Ast::Operator::Equal, 2}},
                        {u8"NotEqual", {Ast::Operator::NotEqual, 2}},
                        {u8"LessThan", {Ast::Operator::Less, 2}},
                        {u8"LessEqual", {Ast::Operator::LessEqual, 2}},
                        {u8"GreaterThan", {Ast::Operator::Greater, 2}},
                        {u8"GreaterEqual", {Ast::Operator::GreaterEqual, 2}},
                        {u8"Is", {Ast::Operator::Is, 2}},

                        // 位运算（一元）
                        {u8"BitNot", {Ast::Operator::BitNot, 1}},

                        // 位运算（二元）
                        {u8"BitAnd", {Ast::Operator::BitAnd, 2}},
                        {u8"BitOr", {Ast::Operator::BitOr, 2}},
                        {u8"BitXor", {Ast::Operator::BitXor, 2}},
                        {u8"ShiftLeft", {Ast::Operator::ShiftLeft, 2}},
                        {u8"ShiftRight", {Ast::Operator::ShiftRight, 2}},
                    };
                    for (auto &implMethod : implementMethods)
                    {
                        const FString &opName = implMethod.name;
                        if (!magic_name_to_op.contains(opName))
                        {
                            // ... 现在忽略
                            // 未来可能报错

                            continue;
                        }

                        auto [op, expectArgCnt] = magic_name_to_op.at(opName);

                        //                                  type          op     isUnary(1-->true, 2-->false)
                        if (ctx->hasOperatorImplemented(structType, op, (expectArgCnt == 1 ? true : false)))
                        {
                            throw EvaluatorError(
                                u8"DuplicateImplementError",
                                std::format("{} has already implement by another interface", opName.toBasicString()),
                                ip);
                        }

                        size_t paraCnt = implMethod.paras.posParas.size(); // 必须为位置参数!
                        if (paraCnt != expectArgCnt || implMethod.paras.size() != expectArgCnt)
                        {
                            // 特化报错，更详细易读
                            throw EvaluatorError(u8"InterfaceSignatureMismatch",
                                                 std::format("Operator {} for {} arg count must be {}, got {}",
                                                             opName.toBasicString(),
                                                             structLv.name().toBasicString(),
                                                             expectArgCnt,
                                                             paraCnt),
                                                 ip);
                        }

                        FString opFnName(u8"Operation." + prettyType(structTypeObj) + u8"." + opName);

                        ContextPtr fnCtx = std::make_shared<Context>(
                            FString(std::format("<Function {}>", opFnName.toBasicString())), ctx);

                        const auto &fillOpFnParas = [this, structType, implMethod, opFnName, fnCtx, ctx, paraCnt](
                                                        const std::vector<ObjectPtr> &args) {
                            const Ast::FunctionParameters &paras = implMethod.paras;
                            for (size_t i = 0; i < paraCnt; ++i)
                            {
                                const TypeInfo &paraType =
                                    actualType(check_unwrap_stres(eval(paras.posParas[i].second, ctx)));
                                if (paraType != ValueType::Any && paraType != structType)
                                {
                                    throw EvaluatorError(
                                        u8"ParameterTypeError",
                                        std::format("Invalid op fn parameter type '{}' of `{}`, must be `{}`",
                                                    paraType.toString().toBasicString(),
                                                    paras.posParas[i].first.toBasicString(),
                                                    structType.toString().toBasicString()),
                                        paras.posParas[i].second);
                                }
                                fnCtx->def(paras.posParas[i].first, paraType, AccessModifier::Normal, args[i]);
                            }
                        };

                        if (paraCnt == 1)
                        {
                            ctx->registerUnaryOperator(structType, op, [=, this](const ObjectPtr &value) -> ExprResult {
                                fillOpFnParas({value});
                                return executeFunction(Function(opFnName,
                                                                implMethod.paras, // parameters
                                                                structType,       // return type --> struct type
                                                                implMethod.body,  // body
                                                                ctx               // closure context
                                                                ),
                                                       Ast::FunctionCallArgs{.argv = {value}},
                                                       fnCtx);
                            });
                        }
                        else
                        {
                            ctx->registerBinaryOperator(
                                structType, op, [=, this](const ObjectPtr &lhs, const ObjectPtr &rhs) {
                                    fillOpFnParas({lhs, rhs});
                                    return executeFunction(Function(opFnName,
                                                                    implMethod.paras, // parameters
                                                                    structType,       // return type --> struct type
                                                                    implMethod.body,  // body
                                                                    ctx               // closure context
                                                                    ),
                                                           Ast::FunctionCallArgs{.argv = {lhs, rhs}},
                                                           fnCtx);
                                });
                        }
                    }
                    return StatementResult::normal();
                }

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

                    ObjectPtr returnTypeValue = check_unwrap_stres(eval(ifMethod.returnType, ctx));

                    record.implMethods[name] =
                        Function(implMethod.name, implMethod.paras, actualType(returnTypeValue), implMethod.body, ctx);
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
                auto ifSt = std::static_pointer_cast<Ast::IfSt>(stmt);
                ObjectPtr condVal = check_unwrap_stres(eval(ifSt->condition, ctx));
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
                    ObjectPtr elifCondVal = check_unwrap_stres(eval(elif->condition, ctx));
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
                auto whileSt = std::static_pointer_cast<Ast::WhileSt>(stmt);
                while (true)
                {
                    ObjectPtr condVal = check_unwrap_stres(eval(whileSt->condition, ctx));
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
                auto forSt = std::static_pointer_cast<Ast::ForSt>(stmt);
                ContextPtr loopContext = std::make_shared<Context>(
                    FString(std::format("<For {}:{}>", forSt->getAAI().line, forSt->getAAI().column)),
                    ctx); // for loop has its own context

                evalStatement(forSt->initSt,
                              loopContext); // ignore init statement result
                size_t iteration = 0;

                while (true) // use while loop to simulate for loop, cause we
                             // need to check condition type every iteration
                {
                    ObjectPtr condVal = check_unwrap_stres(eval(forSt->condition, loopContext));
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
                auto tryst = std::static_pointer_cast<Ast::TrySt>(stmt);

                ContextPtr tryCtx = std::make_shared<Context>(
                    FString(std::format("<Try at {}:{}>", tryst->getAAI().line, tryst->getAAI().column)), ctx);
                StatementResult sr = StatementResult::normal();
                bool crashed = false;
                for (auto &stmt : tryst->body->stmts)
                {
                    sr = evalStatement(stmt, tryCtx); // eval in try context
                    if (sr.isError())
                    {
                        crashed = true;
                        break;
                    }
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
                if (!catched && crashed)
                {
                    throw EvaluatorError(u8"UncaughtExceptionError",
                                         std::format("Uncaught exception: {}", sr.result->toString().toBasicString()),
                                         tryst);
                }
                if (tryst->finallyBlock) { sr = evalBlockStatement(tryst->finallyBlock, ctx); }
                return sr;
            }

            case ThrowSt: {
                auto ts = std::static_pointer_cast<Ast::ThrowSt>(stmt);

                ObjectPtr value = check_unwrap_stres(eval(ts->value, ctx));
                if (value->is<ValueType::NullClass>())
                {
                    throw EvaluatorError(u8"TypeError", u8"Why did you throw a null?", ts);
                }
                return StatementResult::errorFlow(value);
            }

            case ReturnSt: {
                auto returnSt = std::static_pointer_cast<Ast::ReturnSt>(stmt);

                ObjectPtr returnValue = Object::getNullInstance(); // default is null
                if (returnSt->retValue) returnValue = check_unwrap_stres(eval(returnSt->retValue, ctx));
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
                auto exprStmt = std::static_pointer_cast<Ast::ExpressionStmtAst>(stmt);
                return check_unwrap_stres(eval(exprStmt->exp, ctx));
            }

            case BlockStatement: {
                auto block = std::static_pointer_cast<Ast::BlockStatementAst>(stmt);

                ContextPtr blockCtx = std::make_shared<Context>(
                    FString(std::format("<Block at {}:{}>", block->getAAI().line, block->getAAI().column)), ctx);
                return evalBlockStatement(block, blockCtx);
            }

            default:
                throw RuntimeError(
                    FString(std::format("Feature stmt {} unsupported yet", magic_enum::enum_name(stmt->getType()))));
        }
    }
}; // namespace Fig