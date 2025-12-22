#include <evaluator.hpp>
#include <builtins.hpp>
#include <utils.hpp>

namespace Fig
{
    Value Evaluator::__evalOp(Ast::Operator op, const Value &lhs, const Value &rhs)
    {
        using Fig::Ast::Operator;
        switch (op)
        {
            case Operator::Add: return lhs + rhs;
            case Operator::Subtract: return lhs - rhs;
            case Operator::Multiply: return lhs * rhs;
            case Operator::Divide: return lhs / rhs;
            case Operator::Modulo: return lhs % rhs;
            case Operator::Power: return power(lhs, rhs);

            case Operator::And: return lhs && rhs;
            case Operator::Or: return lhs || rhs;
            case Operator::Not: return !lhs;

            case Operator::Equal: return Value(lhs == rhs);
            case Operator::NotEqual: return Value(lhs != rhs);
            case Operator::Less: return lhs < rhs;
            case Operator::LessEqual: return lhs <= rhs;
            case Operator::Greater: return lhs > rhs;
            case Operator::GreaterEqual: return lhs >= rhs;

            case Operator::BitAnd: return bit_and(lhs, rhs);
            case Operator::BitOr: return bit_or(lhs, rhs);
            case Operator::BitXor: return bit_xor(lhs, rhs);
            case Operator::BitNot: return bit_not(lhs);
            case Operator::ShiftLeft: return shift_left(lhs, rhs);
            case Operator::ShiftRight: return shift_right(lhs, rhs);

            // case Operator::Walrus: {
            //     static constexpr char WalrusErrorName[] = "WalrusError";
            //     throw EvaluatorError<WalrusErrorName>(FStringView(u8"Walrus operator is not supported"), currentAddressInfo); // using parent address info for now
            // }
            default:
                throw RuntimeError(FStringView(u8"Unsupported operator"));
        }
    }

    Value Evaluator::evalBinary(const Ast::BinaryExpr &binExp)
    {
        if (binExp->op == Ast::Operator::Dot)
        {
            const Value &lhs = eval(binExp->lexp);
            if (!lhs.is<StructInstance>())
            {
                static constexpr char AccessOpObjectNotStructError[] = "AccessOpObjectNotStructError";
                throw EvaluatorError<AccessOpObjectNotStructError>(FStringView(
                                                                       std::format("Object not a struct")),
                                                                   binExp->lexp->getAAI());
            }
            const StructInstanceT &st = lhs.as<StructInstance>().getValue();
            Ast::VarExpr varExp;
            if (!(varExp = std::dynamic_pointer_cast<Ast::VarExprAst>(binExp->rexp)))
            {
                static constexpr char AccessOpNotAFieldNameError[] = "AccessOpNotAFieldNameError";
                throw EvaluatorError<AccessOpNotAFieldNameError>(FStringView(
                                                                     std::format("{} is not a field name", binExp->rexp->toString().toBasicString())),
                                                                 binExp->rexp->getAAI());
            }
            FString member = varExp->name;
            auto structTypeNameOpt = currentContext->getStructName(st.parentId);
            if (!structTypeNameOpt) throw RuntimeError(FStringView("Can't get struct type name"));
            FString structTypeName = *structTypeNameOpt;
            if (!st.localContext->containsInThisScope(member))
            {
                static constexpr char NoAttributeError[] = "NoAttributeError";
                throw EvaluatorError<NoAttributeError>(FStringView(
                                                           std::format("Struct `{}` has no attribute '{}'", structTypeName.toBasicString(), member.toBasicString())),
                                                       binExp->rexp->getAAI());
            }
            return *st.localContext->get(member); // safe
        }
        if (binExp->op == Ast::Operator::Assign)
        {
            Ast::VarExpr varExp;
            if (!(varExp = std::dynamic_pointer_cast<Ast::VarExprAst>(binExp->rexp)))
            {
                static constexpr char AssignToRightValueError[] = "AssignToRightValueError";
                throw EvaluatorError<AssignToRightValueError>(FStringView(
                                                                  std::format("Can't assign to right value {}", binExp->lexp->toString().toBasicString())),
                                                              binExp->lexp->getAAI());
            }
            const FString& varName = varExp->name;
            if (!currentContext->contains(varName))
            {
                static constexpr char VariableNotFoundErrorName[] = "VariableNotFoundError";
                throw EvaluatorError<VariableNotFoundErrorName>(FStringView(std::format("Variable '{}' not defined", varName.toBasicString())), currentAddressInfo);
            }
        }
        return __evalOp(binExp->op, eval(binExp->lexp), eval(binExp->rexp));
    }
    Value Evaluator::evalUnary(const Ast::UnaryExpr &unExp)
    {
        using Fig::Ast::Operator;
        switch (unExp->op)
        {
            case Operator::Not:
                return !eval(unExp->exp);
            case Operator::Subtract:
                return -eval(unExp->exp);
            case Operator::BitNot:
                return bit_not(eval(unExp->exp));
            default:
                throw RuntimeError(FStringView(std::format("Unsupported unary operator: {}", magic_enum::enum_name(unExp->op))));
        }
    }

    Value Evaluator::evalFunctionCall(const Function &fn, const Ast::FunctionArguments &fnArgs, FString fnName)
    {
        FunctionStruct fnStruct = fn.getValue();
        Ast::FunctionCallArgs evaluatedArgs;
        if (fnStruct.isBuiltin)
        {
            for (const auto &argExpr : fnArgs.argv)
            {
                evaluatedArgs.argv.push_back(eval(argExpr));
            }
            if (fnStruct.builtinParamCount != -1 && fnStruct.builtinParamCount != evaluatedArgs.getLength())
            {
                static constexpr char BuiltinArgumentMismatchErrorName[] = "BuiltinArgumentMismatchError";
                throw EvaluatorError<BuiltinArgumentMismatchErrorName>(FStringView(std::format("Builtin function '{}' expects {} arguments, but {} were provided", fnName.toBasicString(), fnStruct.builtinParamCount, evaluatedArgs.getLength())), currentAddressInfo);
            }
            return fnStruct.builtin(evaluatedArgs.argv);
        }

        // check argument, all types of parameters
        Ast::FunctionParameters fnParas = fnStruct.paras;
        if (fnArgs.getLength() < fnParas.posParas.size() || fnArgs.getLength() > fnParas.size())
        {
            static constexpr char ArgumentMismatchErrorName[] = "ArgumentMismatchError";
            throw EvaluatorError<ArgumentMismatchErrorName>(FStringView(std::format("Function '{}' expects {} to {} arguments, but {} were provided", fnName.toBasicString(), fnParas.posParas.size(), fnParas.size(), fnArgs.getLength())), currentAddressInfo);
        }

        // positional parameters type check
        size_t i;
        for (i = 0; i < fnParas.posParas.size(); i++)
        {
            TypeInfo expectedType(fnParas.posParas[i].second); // look up type info, if exists a type with the name, use it, else throw
            Value argVal = eval(fnArgs.argv[i]);
            TypeInfo actualType = argVal.getTypeInfo();
            if (expectedType != actualType and expectedType != ValueType::Any)
            {
                static constexpr char ArgumentTypeMismatchErrorName[] = "ArgumentTypeMismatchError";
                throw EvaluatorError<ArgumentTypeMismatchErrorName>(FStringView(std::format("In function '{}', argument '{}' expects type '{}', but got type '{}'", fnName.toBasicString(), fnParas.posParas[i].first.toBasicString(), expectedType.toString().toBasicString(), actualType.toString().toBasicString())), currentAddressInfo);
            }
            evaluatedArgs.argv.push_back(argVal);
        }
        // default parameters type check
        for (; i < fnArgs.getLength(); i++)
        {
            size_t defParamIndex = i - fnParas.posParas.size();
            TypeInfo expectedType = fnParas.defParas[defParamIndex].second.first;

            Value defaultVal = eval(fnParas.defParas[defParamIndex].second.second);
            if (expectedType != defaultVal.getTypeInfo() and expectedType != ValueType::Any)
            {
                static constexpr char DefaultParameterTypeErrorName[] = "DefaultParameterTypeError";
                throw EvaluatorError<DefaultParameterTypeErrorName>(FStringView(std::format("In function '{}', default parameter '{}' has type '{}', which does not match the expected type '{}'", fnName.toBasicString(), fnParas.defParas[defParamIndex].first.toBasicString(), defaultVal.getTypeInfo().toString().toBasicString(), expectedType.toString().toBasicString())), currentAddressInfo);
            }

            Value argVal = eval(fnArgs.argv[i]);
            TypeInfo actualType = argVal.getTypeInfo();
            if (expectedType != actualType and expectedType != ValueType::Any)
            {
                static constexpr char ArgumentTypeMismatchErrorName[] = "ArgumentTypeMismatchError";
                throw EvaluatorError<ArgumentTypeMismatchErrorName>(FStringView(std::format("In function '{}', argument '{}' expects type '{}', but got type '{}'", fnName.toBasicString(), fnParas.defParas[defParamIndex].first.toBasicString(), expectedType.toString().toBasicString(), actualType.toString().toBasicString())), currentAddressInfo);
            }
            evaluatedArgs.argv.push_back(argVal);
        }
        // default parameters filling
        for (; i < fnParas.size(); i++)
        {
            size_t defParamIndex = i - fnParas.posParas.size();
            Value defaultVal = eval(fnParas.defParas[defParamIndex].second.second);
            evaluatedArgs.argv.push_back(defaultVal);
        }
        // create new context for function call
        auto newContext = std::make_shared<Context>(FString(std::format("<Function {}()>", fnName.toBasicString())),
                                                    fnStruct.closureContext);
        auto previousContext = currentContext;
        currentContext = newContext;
        // define parameters in new context
        for (size_t j = 0; j < fnParas.size(); j++)
        {
            FString paramName;
            TypeInfo paramType;
            if (j < fnParas.posParas.size())
            {
                paramName = fnParas.posParas[j].first;
                paramType = fnParas.posParas[j].second;
            }
            else
            {
                size_t defParamIndex = j - fnParas.posParas.size();
                paramName = fnParas.defParas[defParamIndex].first;
                paramType = fnParas.defParas[defParamIndex].second.first;
            }
            AccessModifier argAm = AccessModifier::Const;
            currentContext->def(paramName, paramType, argAm, evaluatedArgs.argv[j]);
        }
        // execute function body
        Value retVal = Value::getNullInstance();
        for (const auto &stmt : fnStruct.body->stmts)
        {
            StatementResult sr = evalStatement(stmt);
            if (sr.shouldReturn())
            {
                retVal = sr.result;
                break;
            }
        }
        currentContext = previousContext;
        if (fnStruct.retType != retVal.getTypeInfo() and fnStruct.retType != ValueType::Any)
        {
            static constexpr char ReturnTypeMismatchErrorName[] = "ReturnTypeMismatchError";
            throw EvaluatorError<ReturnTypeMismatchErrorName>(FStringView(std::format("Function '{}' expects return type '{}', but got type '{}'", fnName.toBasicString(), fnStruct.retType.toString().toBasicString(), retVal.getTypeInfo().toString().toBasicString())), currentAddressInfo);
        }
        return retVal;
    }

    Value Evaluator::eval(Ast::Expression exp)
    {
        using Fig::Ast::AstType;
        switch (exp->getType())
        {
            case AstType::ValueExpr: {
                auto valExp = std::dynamic_pointer_cast<Ast::ValueExprAst>(exp);
                return valExp->val;
            }
            case AstType::VarExpr: {
                auto varExp = std::dynamic_pointer_cast<Ast::VarExprAst>(exp);
                auto val = currentContext->get(varExp->name);
                if (val.has_value())
                {
                    return val.value();
                }
                static constexpr char UndefinedVariableErrorName[] = "UndefinedVariableError";
                throw EvaluatorError<UndefinedVariableErrorName>(FStringView(std::format("Variable '{}' is not defined in the current scope", varExp->name.toBasicString())), varExp->getAAI());
            }
            case AstType::BinaryExpr: {
                auto binExp = std::dynamic_pointer_cast<Ast::BinaryExprAst>(exp);
                return evalBinary(binExp);
            }
            case AstType::UnaryExpr: {
                auto unExp = std::dynamic_pointer_cast<Ast::UnaryExprAst>(exp);
                return evalUnary(unExp);
            }
            case AstType::FunctionCall: {
                auto fnCall = std::dynamic_pointer_cast<Ast::FunctionCallExpr>(exp);

                Value calleeVal = eval(fnCall->callee);

                if (!calleeVal.is<Function>())
                {
                    static constexpr char NotAFunctionErrorName[] = "NotAFunctionError";
                    throw EvaluatorError<NotAFunctionErrorName>(
                        FStringView(std::format(
                            "'{}' is not a function or callable",
                            calleeVal.toString().toBasicString())),
                        currentAddressInfo);
                }

                Function fn = calleeVal.as<Function>();

                FString fnName = u8"<anonymous>";
                if (auto var = std::dynamic_pointer_cast<Ast::VarExprAst>(fnCall->callee))
                    fnName = var->name; // try to get function name

                return evalFunctionCall(fn, fnCall->arg, fnName);
            }

            case AstType::FunctionLiteralExpr: {
                auto fn = std::dynamic_pointer_cast<Ast::FunctionLiteralExprAst>(exp);

                if (fn->isExprMode())
                {
                    Ast::BlockStatement body = std::make_shared<Ast::BlockStatementAst>();
                    body->setAAI(fn->getExprBody()->getAAI());
                    Ast::Statement retSt = std::make_shared<Ast::ReturnSt>(fn->getExprBody());
                    retSt->setAAI(fn->getExprBody()->getAAI());
                    body->stmts.push_back(retSt);
                    return Function(
                        fn->paras,
                        ValueType::Any,
                        body,
                        currentContext);
                }
                else
                {
                    Ast::BlockStatement body = fn->getBlockBody();
                    return Function(
                        fn->paras,
                        ValueType::Any,
                        body,
                        currentContext);
                }
            }
            case AstType::InitExpr: {
                auto initExpr = std::dynamic_pointer_cast<Ast::InitExprAst>(exp);
                if (!currentContext->contains(initExpr->structName))
                {
                    static constexpr char StructNotFoundErrorName[] = "StructNotFoundError";
                    throw EvaluatorError<StructNotFoundErrorName>(FStringView(std::format("Structure type '{}' not found", initExpr->structName.toBasicString())), initExpr->getAAI());
                }
                Value structTypeVal = currentContext->get(initExpr->structName).value();
                if (!structTypeVal.is<StructType>())
                {
                    static constexpr char NotAStructTypeErrorName[] = "NotAStructTypeError";
                    throw EvaluatorError<NotAStructTypeErrorName>(FStringView(std::format("'{}' is not a structure type", initExpr->structName.toBasicString())), initExpr->getAAI());
                }
                const StructT &structT = structTypeVal.as<StructType>().getValue();
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
                    static constexpr char StructInitArgumentMismatchErrorName[] = "StructInitArgumentMismatchError";
                    throw EvaluatorError<StructInitArgumentMismatchErrorName>(FStringView(std::format("Structure '{}' expects {} to {} fields, but {} were provided", initExpr->structName.toBasicString(), minArgs, maxArgs, initExpr->args.size())), initExpr->getAAI());
                }

                std::vector<std::pair<FString, Value>> evaluatedArgs;
                for (const auto &[argName, argExpr] : initExpr->args)
                {
                    evaluatedArgs.push_back({argName, eval(argExpr)});
                }
                ContextPtr instanceCtx = std::make_shared<Context>(
                    FString(std::format("<StructInstance {}>", initExpr->structName.toBasicString())),
                    currentContext);
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
                                // we've checked argument count before, so here must be a default value
                                ContextPtr previousContext = currentContext;
                                currentContext = defContext; // evaluate default value in definition context

                                Value defaultVal = eval(field.defaultValue); // it can't be null here

                                currentContext = previousContext;

                                // type check
                                if (expectedType != defaultVal.getTypeInfo() && expectedType != ValueType::Any)
                                {
                                    static constexpr char StructFieldTypeMismatchErrorName[] = "StructFieldTypeMismatchError";
                                    throw EvaluatorError<StructFieldTypeMismatchErrorName>(FStringView(std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'", initExpr->structName.toBasicString(), fieldName.toBasicString(), expectedType.toString().toBasicString(), defaultVal.getTypeInfo().toString().toBasicString())), initExpr->getAAI());
                                }

                                instanceCtx->def(fieldName, expectedType, field.am, defaultVal);
                                continue;
                            }

                            const Value &argVal = evaluatedArgs[i].second;
                            if (expectedType != argVal.getTypeInfo() && expectedType != ValueType::Any)
                            {
                                static constexpr char StructFieldTypeMismatchErrorName[] = "StructFieldTypeMismatchError";
                                throw EvaluatorError<StructFieldTypeMismatchErrorName>(FStringView(std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'", initExpr->structName.toBasicString(), fieldName.toBasicString(), expectedType.toString().toBasicString(), argVal.getTypeInfo().toString().toBasicString())), initExpr->getAAI());
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
                                static constexpr char StructFieldRedeclarationErrorName[] = "StructFieldRedeclarationError";
                                throw EvaluatorError<StructFieldRedeclarationErrorName>(FStringView(std::format("Field '{}' already initialized in structure '{}'", fieldName.toBasicString(), initExpr->structName.toBasicString())), initExpr->getAAI());
                            }
                            if (i + 1 > got)
                            {
                                // use default value
                                ContextPtr previousContext = currentContext;
                                currentContext = defContext;                 // evaluate default value in definition context
                                Value defaultVal = eval(field.defaultValue); // it can't be null here
                                currentContext = previousContext;

                                // type check
                                const TypeInfo &expectedType = field.type;
                                if (expectedType != defaultVal.getTypeInfo() && expectedType != ValueType::Any)
                                {
                                    static constexpr char StructFieldTypeMismatchErrorName[] = "StructFieldTypeMismatchError";
                                    throw EvaluatorError<StructFieldTypeMismatchErrorName>(FStringView(std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'", initExpr->structName.toBasicString(), fieldName.toBasicString(), expectedType.toString().toBasicString(), defaultVal.getTypeInfo().toString().toBasicString())), initExpr->getAAI());
                                }

                                instanceCtx->def(fieldName, field.type, field.am, defaultVal);
                                continue;
                            }
                            const Value &argVal = evaluatedArgs[i].second;
                            if (field.type != argVal.getTypeInfo() && field.type != ValueType::Any)
                            {
                                static constexpr char StructFieldTypeMismatchErrorName[] = "StructFieldTypeMismatchError";
                                throw EvaluatorError<StructFieldTypeMismatchErrorName>(FStringView(std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'", initExpr->structName.toBasicString(), fieldName.toBasicString(), field.type.toString().toBasicString(), argVal.getTypeInfo().toString().toBasicString())), initExpr->getAAI());
                            }
                            instanceCtx->def(fieldName, field.type, field.am, argVal);
                        }
                    }
                }
                return StructInstance(structT.id, instanceCtx);
            }
            default:
                throw RuntimeError(FStringView("Unknown expression type:" + std::to_string(static_cast<int>(exp->getType()))));
                return Value::getNullInstance();
        }
    }
    StatementResult Evaluator::evalBlockStatement(const Ast::BlockStatement &blockSt, ContextPtr context)
    {
        auto previousContext = currentContext;
        if (context)
        {
            currentContext = context;
        }
        else
        {
            currentContext = std::make_shared<Context>(FString(std::format("<Block {}:{}>", blockSt->getAAI().line, blockSt->getAAI().column)), currentContext);
        }
        StatementResult lstResult = StatementResult::normal();
        for (const auto &s : blockSt->stmts)
        {
            StatementResult sr = evalStatement(s);
            if (!sr.isNormal())
            {
                lstResult = sr;
                break;
            }
        }
        currentContext = previousContext;
        return lstResult;
    }
    StatementResult Evaluator::evalStatement(const Ast::Statement &stmt)
    {
        using Fig::Ast::AstType;
        currentAddressInfo = stmt->getAAI();
        switch (stmt->getType())
        {
            case AstType::VarDefSt: {
                auto varDef = std::dynamic_pointer_cast<Ast::VarDefAst>(stmt);
                if (currentContext->contains(varDef->name))
                {
                    static constexpr char RedeclarationErrorName[] = "RedeclarationError";
                    throw EvaluatorError<RedeclarationErrorName>(FStringView(std::format("Variable '{}' already defined in this scope", varDef->name.toBasicString())), currentAddressInfo);
                }
                Value val;
                TypeInfo varTypeInfo;
                if (varDef->typeName == Parser::varDefTypeFollowed)
                {
                    // has expr
                    val = eval(varDef->expr);
                    varTypeInfo = val.getTypeInfo();
                }
                else if (varDef->expr)
                {
                    val = eval(varDef->expr);
                    if (varDef->typeName != ValueType::Any.name)
                    {
                        TypeInfo expectedType(varDef->typeName);
                        TypeInfo actualType = val.getTypeInfo();
                        if (expectedType != actualType and expectedType != ValueType::Any)
                        {
                            static constexpr char VariableTypeMismatchErrorName[] = "VariableTypeMismatchError";
                            throw EvaluatorError<VariableTypeMismatchErrorName>(FStringView(std::format("Variable '{}' expects type '{}', but got type '{}'", varDef->name.toBasicString(), expectedType.toString().toBasicString(), actualType.toString().toBasicString())), varDef->getAAI());
                        }
                    }
                }
                else if (!varDef->typeName.empty())
                {
                    varTypeInfo = TypeInfo(varDef->typeName); // may throw
                    val = Value::defaultValue(varTypeInfo);
                }
                AccessModifier am = (varDef->isPublic ? (varDef->isConst ? AccessModifier::PublicConst : AccessModifier::Public) : (varDef->isConst ? AccessModifier::Const : AccessModifier::Normal));
                currentContext->def(varDef->name, varTypeInfo, am, val);
                return StatementResult::normal();
            }
            case AstType::ExpressionStmt: {
                auto exprSt = std::dynamic_pointer_cast<Ast::ExpressionStmtAst>(stmt);
                eval(exprSt->exp);
                return StatementResult::normal();
            };
            case AstType::BlockStatement: {
                auto blockSt = std::dynamic_pointer_cast<Ast::BlockStatementAst>(stmt);
                return evalBlockStatement(blockSt); // auto create new context in block statement
            };
            case AstType::FunctionDefSt: {
                auto fnDef = std::dynamic_pointer_cast<Ast::FunctionDefSt>(stmt);
                if (currentContext->contains(fnDef->name))
                {
                    static constexpr char RedeclarationErrorName[] = "RedeclarationError";
                    throw EvaluatorError<RedeclarationErrorName>(FStringView(std::format("Function '{}' already defined in this scope", fnDef->name.toBasicString())), currentAddressInfo);
                }
                AccessModifier am = (fnDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const);
                currentContext->def(
                    fnDef->name,
                    ValueType::Function,
                    am,
                    Value(Function(
                        fnDef->paras,
                        TypeInfo(fnDef->retType),
                        fnDef->body,
                        currentContext)));
                return StatementResult::normal();
            };
            case AstType::StructSt: {
                auto stDef = std::dynamic_pointer_cast<Ast::StructDefSt>(stmt);
                if (currentContext->containsInThisScope(stDef->name))
                {
                    static constexpr char RedeclarationErrorName[] = "RedeclarationError";
                    throw EvaluatorError<RedeclarationErrorName>(FStringView(std::format("Structure '{}' already defined in this scope", stDef->name.toBasicString())), currentAddressInfo);
                }
                std::vector<Field> fields;
                std::vector<FString> _fieldNames;
                for (Ast::StructDefField field : stDef->fields)
                {
                    if (Utils::vectorContains(field.fieldName, _fieldNames))
                    {
                        static constexpr char RedeclarationErrorName[] = "RedeclarationError";
                        throw EvaluatorError<RedeclarationErrorName>(FStringView(std::format("Field '{}' already defined in structure '{}'", field.fieldName.toBasicString(), stDef->name.toBasicString())), currentAddressInfo);
                    }
                    fields.push_back(Field(field.am, field.fieldName, TypeInfo(field.tiName), field.defaultValueExpr));
                }
                ContextPtr defContext(currentContext);
                AccessModifier am = (stDef->isPublic ? AccessModifier::PublicConst : AccessModifier::Const);
                TypeInfo _(stDef->name, true); // register type name
                currentContext->def(
                    stDef->name,
                    ValueType::StructType,
                    am,
                    Value(StructType(
                        defContext,
                        fields)));
                return StatementResult::normal();
            }
            // case AstType::VarAssignSt: {
            //     auto varAssign = std::dynamic_pointer_cast<Ast::VarAssignSt>(stmt);
            //     if (!currentContext->contains(varAssign->varName))
            //     {
            //         static constexpr char VariableNotFoundErrorName[] = "VariableNotFoundError";
            //         throw EvaluatorError<VariableNotFoundErrorName>(FStringView(std::format("Variable '{}' not defined", varAssign->varName.toBasicString())), currentAddressInfo);
            //     }
            //     if (!currentContext->isVariableMutable(varAssign->varName))
            //     {
            //         static constexpr char ConstAssignmentErrorName[] = "ConstAssignmentError";
            //         throw EvaluatorError<ConstAssignmentErrorName>(FStringView(std::format("Cannot assign to constant variable '{}'", varAssign->varName.toBasicString())), currentAddressInfo);
            //     }
            //     Value val = eval(varAssign->valueExpr);
            //     if (currentContext->getTypeInfo(varAssign->varName) != ValueType::Any)
            //     {
            //         TypeInfo expectedType = currentContext->getTypeInfo(varAssign->varName);
            //         TypeInfo actualType = val.getTypeInfo();
            //         if (expectedType != actualType)
            //         {
            //             static constexpr char VariableTypeMismatchErrorName[] = "VariableTypeMismatchError";
            //             throw EvaluatorError<VariableTypeMismatchErrorName>(FStringView(std::format("assigning: Variable '{}' expects type '{}', but got type '{}'", varAssign->varName.toBasicString(), expectedType.toString().toBasicString(), actualType.toString().toBasicString())), currentAddressInfo);
            //         }
            //     }
            //     currentContext->set(varAssign->varName, val);
            //     return StatementResult::normal();
            // };
            case AstType::IfSt: {
                auto ifSt = std::dynamic_pointer_cast<Ast::IfSt>(stmt);
                Value condVal = eval(ifSt->condition);
                if (condVal.getTypeInfo() != ValueType::Bool)
                {
                    static constexpr char ConditionTypeErrorName[] = "ConditionTypeError";
                    throw EvaluatorError<ConditionTypeErrorName>(FStringView(u8"If condition must be boolean"), currentAddressInfo);
                }
                if (condVal.as<Bool>().getValue())
                {
                    return evalBlockStatement(ifSt->body);
                }
                // else
                for (const auto &elif : ifSt->elifs)
                {
                    Value elifCondVal = eval(elif->condition);
                    if (elifCondVal.getTypeInfo() != ValueType::Bool)
                    {
                        static constexpr char ConditionTypeErrorName[] = "ConditionTypeError";
                        throw EvaluatorError<ConditionTypeErrorName>(FStringView(u8"Else-if condition must be boolean"), currentAddressInfo);
                    }
                    if (elifCondVal.as<Bool>().getValue())
                    {
                        return evalBlockStatement(elif->body);
                    }
                }
                if (ifSt->els)
                {
                    return evalBlockStatement(ifSt->els->body);
                }
                return StatementResult::normal();
            };
            case AstType::WhileSt: {
                auto whileSt = std::dynamic_pointer_cast<Ast::WhileSt>(stmt);
                while (true)
                {
                    Value condVal = eval(whileSt->condition);
                    if (condVal.getTypeInfo() != ValueType::Bool)
                    {
                        static constexpr char ConditionTypeErrorName[] = "ConditionTypeError";
                        throw EvaluatorError<ConditionTypeErrorName>(FStringView(u8"While condition must be boolean"), whileSt->condition->getAAI());
                    }
                    if (!condVal.as<Bool>().getValue())
                    {
                        break;
                    }
                    ContextPtr loopContext = std::make_shared<Context>(
                        FString(std::format("<While {}:{}>",
                                            whileSt->getAAI().line, whileSt->getAAI().column)),
                        currentContext); // every loop has its own context
                    StatementResult sr = evalBlockStatement(whileSt->body, loopContext);
                    if (sr.shouldReturn())
                    {
                        return sr;
                    }
                    if (sr.shouldBreak())
                    {
                        break;
                    }
                    if (sr.shouldContinue())
                    {
                        continue;
                    }
                }
                return StatementResult::normal();
            };
            case AstType::ForSt: {
                auto forSt = std::dynamic_pointer_cast<Ast::ForSt>(stmt);
                ContextPtr loopContext = std::make_shared<Context>(
                    FString(std::format("<For {}:{}>",
                                        forSt->getAAI().line, forSt->getAAI().column)),
                    currentContext); // for loop has its own context
                ContextPtr previousContext = currentContext;
                currentContext = loopContext;

                evalStatement(forSt->initSt); // ignore init statement result
                size_t iteration = 0;

                while (true) // use while loop to simulate for loop, cause we need to check condition type every iteration
                {
                    Value condVal = eval(forSt->condition);
                    if (condVal.getTypeInfo() != ValueType::Bool)
                    {
                        static constexpr char ConditionTypeErrorName[] = "ConditionTypeError";
                        throw EvaluatorError<ConditionTypeErrorName>(FStringView(u8"For condition must be boolean"), forSt->condition->getAAI());
                    }
                    if (!condVal.as<Bool>().getValue())
                    {
                        break;
                    }
                    iteration++;
                    ContextPtr iterationContext = std::make_shared<Context>(
                        FString(std::format("<For {}:{}, Iteration {}>",
                                            forSt->getAAI().line, forSt->getAAI().column, iteration)),
                        loopContext); // every loop has its own context
                    StatementResult sr = evalBlockStatement(forSt->body, iterationContext);
                    if (sr.shouldReturn())
                    {
                        currentContext = previousContext; // restore context before return
                        return sr;
                    }
                    if (sr.shouldBreak())
                    {
                        break;
                    }
                    if (sr.shouldContinue())
                    {
                        // continue to next iteration
                        continue;
                    }
                    currentContext = loopContext;      // let increment statement be in loop context
                    evalStatement(forSt->incrementSt); // ignore increment statement result
                }
                currentContext = previousContext; // restore context
                return StatementResult::normal();
            }
            case AstType::ReturnSt: {
                if (!currentContext->parent)
                {
                    static constexpr char ReturnOutsideFunctionErrorName[] = "ReturnOutsideFunctionError";
                    throw EvaluatorError<ReturnOutsideFunctionErrorName>(FStringView(u8"'return' statement outside function"), currentAddressInfo);
                }
                if (!currentContext->isInFunctionContext())
                {
                    static constexpr char ReturnOutsideFunctionErrorName[] = "ReturnOutsideFunctionError";
                    throw EvaluatorError<ReturnOutsideFunctionErrorName>(FStringView(u8"'return' statement outside function"), currentAddressInfo);
                }
                auto returnSt = std::dynamic_pointer_cast<Ast::ReturnSt>(stmt);
                return StatementResult::returnFlow(eval(returnSt->retValue));
            };
            case AstType::BreakSt: {
                if (!currentContext->parent)
                {
                    static constexpr char BreakOutsideLoopErrorName[] = "BreakOutsideLoopError";
                    throw EvaluatorError<BreakOutsideLoopErrorName>(FStringView(u8"'break' statement outside loop"), currentAddressInfo);
                }
                if (!currentContext->isInLoopContext())
                {
                    static constexpr char BreakOutsideLoopErrorName[] = "BreakOutsideLoopError";
                    throw EvaluatorError<BreakOutsideLoopErrorName>(FStringView(u8"'break' statement outside loop"), currentAddressInfo);
                }
                return StatementResult::breakFlow();
            };
            case AstType::ContinueSt: {
                if (!currentContext->parent)
                {
                    static constexpr char ContinueOutsideLoopErrorName[] = "ContinueOutsideLoopError";
                    throw EvaluatorError<ContinueOutsideLoopErrorName>(FStringView(u8"'continue' statement outside loop"), currentAddressInfo);
                }
                if (!currentContext->isInLoopContext())
                {
                    static constexpr char ContinueOutsideLoopErrorName[] = "ContinueOutsideLoopError";
                    throw EvaluatorError<ContinueOutsideLoopErrorName>(FStringView(u8"'continue' statement outside loop"), currentAddressInfo);
                }
                return StatementResult::continueFlow();
            };
            default:
                throw RuntimeError(FStringView(std::string("Unknown statement type:") + magic_enum::enum_name(stmt->getType()).data()));
        }
        return StatementResult::normal();
    }

    void Evaluator::run()
    {
        for (auto ast : asts)
        {
            currentAddressInfo = ast->getAAI();
            if (std::dynamic_pointer_cast<Ast::ExpressionStmtAst>(ast))
            {
                auto exprAst = std::dynamic_pointer_cast<Ast::ExpressionStmtAst>(ast);
                Ast::Expression exp = exprAst->exp;
                eval(exp);
            }
            else if (dynamic_cast<Ast::StatementAst *>(ast.get()))
            {
                auto stmtAst = std::dynamic_pointer_cast<Ast::StatementAst>(ast);
                evalStatement(stmtAst);
            }
            else
            {
                throw RuntimeError(FStringView(u8"Unknown AST type"));
            }
        }
    }

    void Evaluator::printStackTrace() const
    {
        if (currentContext)
            currentContext->printStackTrace();
        else
            std::cerr << "[STACK TRACE] (No context has been loaded)\n";
    }
} // namespace Fig