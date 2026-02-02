#include "Evaluator/Value/value.hpp"
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{
    RvObject Evaluator::evalInitExpr(Ast::InitExpr initExpr, ContextPtr ctx)
    {
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
                throw EvaluatorError(u8"StructInitArgumentMismatchError",
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
                throw EvaluatorError(u8"BuiltinInitTypeMismatchError",
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

        auto evalArguments = [&evaluatedArgs, initExpr, ctx, this]() {
            for (const auto &[argName, argExpr] : initExpr->args)
            {
                evaluatedArgs.push_back({argName, eval(argExpr, ctx)});
            }
        };

        ContextPtr instanceCtx =
            std::make_shared<Context>(FString(std::format("<StructInstance {}>", structName.toBasicString())), ctx);
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
                evalArguments();

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
                                std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'",
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
            else if (initExpr->initMode == Named)
            {
                evalArguments();

                // named
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
                                std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'",
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
            else
            {
                // shorthand, can be unordered
                // in this mode, initExpr args are all VarExpr
                // field name is the variable name
                for (const auto &[argName, argExpr] : initExpr->args)
                {
                    // assert(argExpr->getType() == Ast::AstType::VarExpr);
                    // argName is var name
                    const ObjectPtr &argVal = eval(argExpr, ctx); // get the value
                    // find field
                    auto fieldIt = std::find_if(structT.fields.begin(),
                                                structT.fields.end(),
                                                [&argName](const Field &f) { return f.name == argName; });
                    if (fieldIt == structT.fields.end())
                    {
                        // throw EvaluatorError(u8"StructFieldNotFoundError",
                        //                      std::format("Field '{}' not found in structure '{}'",
                        //                                  argName.toBasicString(),
                        //                                  structName.toBasicString()),
                        //                      initExpr);
                        initExpr->initMode = Positional;
                        return evalInitExpr(initExpr, ctx);
                    }
                    const Field &field = *fieldIt;
                    if (!isTypeMatch(field.type, argVal, ctx))
                    {
                        throw EvaluatorError(
                            u8"StructFieldTypeMismatchError",
                            std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'",
                                        structName.toBasicString(),
                                        field.name.toBasicString(),
                                        field.type.toString().toBasicString(),
                                        prettyType(argVal).toBasicString()),
                            initExpr);
                    }
                    // field.name is argName (var name)
                    // Point{x=x, y=y} --> Point{x, y}

                    instanceCtx->def(field.name, field.type, field.am, argVal);
                }
                // fill default values
                size_t currentFieldCount =
                    initExpr->args.size(); // we have already check argument count, min <= got <= max
                // so remain fields start from currentFieldCount to maxArgs
                for (size_t i = currentFieldCount; i < maxArgs; ++i)
                {
                    const Field &field = structT.fields[i];

                    // evaluate default value in definition context
                    ObjectPtr defaultVal = eval(field.defaultValue,
                                                defContext); // it can't be null here

                    // type check
                    if (!isTypeMatch(field.type, defaultVal, ctx))
                    {
                        throw EvaluatorError(
                            u8"StructFieldTypeMismatchError",
                            std::format("In structure '{}', field '{}' expects type '{}', but got type '{}'",
                                        structName.toBasicString(),
                                        field.name.toBasicString(),
                                        field.type.toString().toBasicString(),
                                        prettyType(defaultVal).toBasicString()),
                            initExpr);
                    }

                    instanceCtx->def(field.name, field.type, field.am, defaultVal);
                }
            }
        }
        ContextPtr stDefCtx = structT.defContext;

        // load struct method
        for (auto &[id, fn] : stDefCtx->getFunctions())
        {
            const FString &funcName = fn.name;
            const auto &funcSlot = stDefCtx->get(funcName);
            
            instanceCtx->def(funcName,
                             ValueType::Function,
                             funcSlot->am,
                             std::make_shared<Object>(Function(funcName, fn.paras, fn.retType, fn.body, instanceCtx)));
        }

        return std::make_shared<Object>(StructInstance(structT.type, instanceCtx));
    }
};