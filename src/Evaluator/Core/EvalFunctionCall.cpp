#include "Ast/functionParameters.hpp"
#include "Evaluator/Value/value.hpp"
#include <Ast/Expressions/FunctionCall.hpp>
#include <Evaluator/Value/function.hpp>
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{
    RvObject Evaluator::executeFunction(const Function &fn,
                                        const Ast::FunctionCallArgs &args,
                                        ContextPtr fnCtx) // new context for fn, already filled paras
    {
        // const FString &fnName = fn.name;
        if (fn.type == Function::Builtin || fn.type == Function::MemberType)
        {
            if (fn.type == Function::Builtin) { return fn.builtin(args.argv); }
            else
            {
                return fn.mtFn(nullptr,
                               args.argv); // wrapped member type function (`this` provided by evalMemberExpr)
            }
        }
        // else: normal fn, args is needless
        for (const auto &stmt : fn.body->stmts)
        {
            StatementResult sr = evalStatement(stmt, fnCtx);
            if (sr.isError())
            {
                throw EvaluatorError(u8"UncaughtExceptionError",
                                     std::format("Uncaught exception: {}", sr.result->toString().toBasicString()),
                                     stmt);
            }
            if (!sr.isNormal())
            {
                return sr.result;
            }
        }
        return Object::getNullInstance();
    }
    RvObject Evaluator::evalFunctionCall(const Ast::FunctionCall &call, ContextPtr ctx)
    {
        RvObject fnObj = eval(call->callee, ctx);
        if (fnObj->getTypeInfo() != ValueType::Function)
        {
            throw EvaluatorError(u8"ObjectNotCallable",
                                 std::format("Object `{}` isn't callable", fnObj->toString().toBasicString()),
                                 call->callee);
        }

        const Function &fn = fnObj->as<Function>();

        const FString &fnName = fn.name;
        const Ast::FunctionArguments &fnArgs = call->arg;

        Ast::FunctionCallArgs evaluatedArgs;
        if (fn.type == Function::Builtin || fn.type == Function::MemberType)
        {
            for (const auto &argExpr : fnArgs.argv) { evaluatedArgs.argv.push_back(eval(argExpr, ctx)); }
            if (fn.builtinParamCount != -1 && fn.builtinParamCount != evaluatedArgs.getLength())
            {
                throw EvaluatorError(u8"BuiltinArgumentMismatchError",
                                     std::format("Builtin function '{}' expects {} arguments, but {} were provided",
                                                 fnName.toBasicString(),
                                                 fn.builtinParamCount,
                                                 evaluatedArgs.getLength()),
                                     fnArgs.argv.back());
            }
            return executeFunction(fn, evaluatedArgs, nullptr);
        }

        // check argument, all types of parameters
        Ast::FunctionParameters fnParas = fn.paras;

        // create new context for function call
        auto newContext = std::make_shared<Context>(FString(std::format("<Function {}()>", fnName.toBasicString())),
                                                    fn.closureContext);

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
            const TypeInfo &expectedType = actualType(eval(fnParas.posParas[i].second, ctx)); // look up type info, if exists a type
                                                                                  // with the name, use it, else throw
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
            const TypeInfo &expectedType = actualType(eval(fnParas.defParas[defParamIndex].second.first, ctx));

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
                paramType = actualType(eval(fnParas.posParas[j].second, ctx));
            }
            else
            {
                size_t defParamIndex = j - fnParas.posParas.size();
                paramName = fnParas.defParas[defParamIndex].first;
                paramType = actualType(eval(fnParas.defParas[defParamIndex].second.first, ctx));
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
        ObjectPtr retVal = executeFunction(fn, evaluatedArgs, newContext);

        if (!isTypeMatch(fn.retType, retVal, ctx))
        {
            throw EvaluatorError(u8"ReturnTypeMismatchError",
                                 std::format("Function '{}' expects return type '{}', but got type '{}'",
                                             fnName.toBasicString(),
                                             fn.retType.toString().toBasicString(),
                                             prettyType(retVal).toBasicString()),
                                 fn.body);
        }
        return retVal;
    }
    }
}; // namespace Fig