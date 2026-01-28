#include <Evaluator/Value/function.hpp>
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{
    RvObject Evaluator::evalFunctionCall(const Function &fn,
                                         const Ast::FunctionArguments &fnArgs,
                                         const FString &fnName,
                                         ContextPtr ctx)
    {
        const Function &fnStruct = fn;
        Ast::FunctionCallArgs evaluatedArgs;
        if (fnStruct.type == Function::Builtin || fnStruct.type == Function::MemberType)
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
            if (fnStruct.type == Function::Builtin) 
            {
                return fnStruct.builtin(evaluatedArgs.argv);
            }
            else
            {
                return fnStruct.mtFn(nullptr, evaluatedArgs.argv); // wrapped member type function (`this` provided by evalMemberExpr)
            }
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
            AccessModifier argAm = AccessModifier::Normal;
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
        newContext->def(fnParas.variadicPara, ValueType::List, AccessModifier::Normal, std::make_shared<Object>(list));
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
}; // namespace Fig