#include <Ast/functionParameters.hpp>
#include <Evaluator/Value/value.hpp>
#include <Ast/Expressions/FunctionCall.hpp>
#include <Evaluator/Value/function.hpp>
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>
#include <Evaluator/Core/ExprResult.hpp>

namespace Fig
{
   ExprResult Evaluator::executeFunction(const Function &fn,
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
                handle_error(sr, stmt, fnCtx);
            }
            if (!sr.isNormal())
            {
                return sr.result;
            }
        }
        return Object::getNullInstance();
    }
    ExprResult Evaluator::evalFunctionCall(const Ast::FunctionCall &call, ContextPtr ctx)
    {
        RvObject fnObj = check_unwrap(eval(call->callee, ctx));
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
            for (const auto &argExpr : fnArgs.argv) { evaluatedArgs.argv.push_back(check_unwrap(eval(argExpr, ctx))); }
            if (fn.builtinParamCount != -1 && fn.builtinParamCount != evaluatedArgs.getLength())
            {
                throw EvaluatorError(u8"BuiltinArgumentMismatchError",
                                     std::format("Builtin function '{}' expects {} arguments, but {} were provided",
                                                 fnName.toBasicString(),
                                                 fn.builtinParamCount,
                                                 evaluatedArgs.getLength()),
                                     (fnArgs.getLength() > 0 ? fnArgs.argv.back() : call));
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
            const TypeInfo &expectedType = actualType(check_unwrap(eval(fnParas.posParas[i].second, fn.closureContext))); // look up type info, if exists a type
                                                                                  // with the name, use it, else throw
            ObjectPtr argVal = check_unwrap(eval(fnArgs.argv[i], ctx));
            TypeInfo actualType = argVal->getTypeInfo();
            if (!isTypeMatch(expectedType, argVal, fn.closureContext))
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
            const TypeInfo &expectedType =
                actualType(check_unwrap(eval(fnParas.defParas[defParamIndex].second.first, fn.closureContext)));

            ObjectPtr defaultVal = check_unwrap(eval(fnParas.defParas[defParamIndex].second.second, fn.closureContext));
            if (!isTypeMatch(expectedType, defaultVal, fn.closureContext))
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

            ObjectPtr argVal = check_unwrap(eval(fnArgs.argv[i], ctx));
            TypeInfo actualType = argVal->getTypeInfo();
            if (!isTypeMatch(expectedType, argVal, fn.closureContext))
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
            ObjectPtr defaultVal = check_unwrap(eval(fnParas.defParas[defParamIndex].second.second, fn.closureContext));
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
                paramType = actualType(check_unwrap(eval(fnParas.posParas[j].second, fn.closureContext)));
            }
            else
            {
                size_t defParamIndex = j - fnParas.posParas.size();
                paramName = fnParas.defParas[defParamIndex].first;
                paramType =
                    actualType(check_unwrap(eval(fnParas.defParas[defParamIndex].second.first, fn.closureContext)));
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
            list.push_back(check_unwrap(eval(exp, ctx))); // eval arguments in current scope
        }
        newContext->def(fnParas.variadicPara, ValueType::List, AccessModifier::Normal, std::make_shared<Object>(list));
        goto ExecuteBody;
    }

    ExecuteBody: {
        // execute function body
        ObjectPtr retVal = check_unwrap(executeFunction(fn, evaluatedArgs, newContext));

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