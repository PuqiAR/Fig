#include <Ast/Expressions/BinaryExpr.hpp>
#include <Ast/Expressions/FunctionCall.hpp>
#include <Ast/Expressions/ValueExpr.hpp>
#include <Ast/Expressions/VarExpr.hpp>
#include <Ast/Statements/ControlSt.hpp>
#include <Ast/astBase.hpp>
#include <Ast/functionParameters.hpp>
#include <Evaluator/Context/context.hpp>
#include <Core/fig_string.hpp>
#include <Ast/AccessModifier.hpp>
#include <Evaluator/Value/structType.hpp>
#include <Evaluator/Value/value.hpp>
#include <Module/builtins.hpp>

#include <memory>
#include <print>
#include <iostream>
#include <cmath>
#include <chrono>
#include <numeric>
#include <unordered_map>

namespace Fig::Builtins
{
    const TypeInfo &getErrorInterfaceTypeInfo()
    {
        static const TypeInfo ErrorInterfaceTypeInfo(u8"Error", true);
        return ErrorInterfaceTypeInfo;
    }
    const TypeInfo &getTypeErrorStructTypeInfo()
    {
        static const TypeInfo TypeErrorStructTypeInfo(u8"TypeError", true);
        return TypeErrorStructTypeInfo;
    }
    const TypeInfo &getOperationInterfaceTypeInfo()
    {
        static const TypeInfo OperationInterfaceTypeInfo(u8"Operation", true);
        return OperationInterfaceTypeInfo;
    }
    const std::unordered_map<FString, ObjectPtr> &getBuiltinValues()
    {
        static const std::unordered_map<FString, ObjectPtr> builtinValues = {
            {u8"null", Object::getNullInstance()},
            {u8"true", Object::getTrueInstance()},
            {u8"false", Object::getFalseInstance()},
            {u8"Error",
             std::make_shared<Object>(InterfaceType(getErrorInterfaceTypeInfo(),
                                                    {Ast::InterfaceMethod(u8"toString",
                                                                          Ast::FunctionParameters({}, {}),
                                                                          std::make_shared<Ast::VarExprAst>(u8"String"),
                                                                          nullptr),
                                                     Ast::InterfaceMethod(u8"getErrorClass",
                                                                          Ast::FunctionParameters({}, {}),
                                                                          std::make_shared<Ast::VarExprAst>(u8"String"),
                                                                          nullptr),
                                                     Ast::InterfaceMethod(u8"getErrorMessage",
                                                                          Ast::FunctionParameters({}, {}),
                                                                          std::make_shared<Ast::VarExprAst>(u8"String"),
                                                                          nullptr)}))},
                {u8"TypeError", std::make_shared<Object>(StructType(
                    getTypeErrorStructTypeInfo(),
                    std::make_shared<Context>(u8"<Built-in `TypeError`>"),
                    {Field(AccessModifier::Public, u8"msg", ValueType::String, nullptr)}
                ))},
            {u8"Operation", std::make_shared<Object>(InterfaceType(getOperationInterfaceTypeInfo(), {}))},

            {u8"Any", std::make_shared<Object>(StructType(ValueType::Any, nullptr, {}, true))},
            {u8"Int", std::make_shared<Object>(StructType(ValueType::Int, nullptr, {}, true))},
            {u8"Null", std::make_shared<Object>(StructType(ValueType::Null, nullptr, {}, true))},
            {u8"String", std::make_shared<Object>(StructType(ValueType::String, nullptr, {}, true))},
            {u8"Bool", std::make_shared<Object>(StructType(ValueType::Bool, nullptr, {}, true))},
            {u8"Double", std::make_shared<Object>(StructType(ValueType::Double, nullptr, {}, true))},
            {u8"Function", std::make_shared<Object>(StructType(ValueType::Function, nullptr, {}, true))},
            {u8"List", std::make_shared<Object>(StructType(ValueType::List, nullptr, {}, true))},
            {u8"Map", std::make_shared<Object>(StructType(ValueType::Map, nullptr, {}, true))},
            // Type `StructType` `StructInstance` `Module` `InterfaceType`
            // Not allowed to call constructor!

            {u8"type", std::make_shared<Object>(Function(
                u8"type",
                [](const std::vector<ObjectPtr> &_args) -> ObjectPtr
                {
                    const ObjectPtr &arg = _args[0];
                    return std::make_shared<Object>(prettyType(arg));
                },
                1
            ))},
        };
        return builtinValues;
    }
    const std::unordered_map<FString, int> &getBuiltinFunctionArgCounts()
    {
        static const std::unordered_map<FString, int> builtinFunctionArgCounts = {
            {u8"__fstdout_print", -1},   // variadic
            {u8"__fstdout_println", -1}, // variadic
            {u8"__fstdin_read", 0},
            {u8"__fstdin_readln", 0},
            {u8"__fvalue_type", 1},
            {u8"__fvalue_int_parse", 1},
            {u8"__fvalue_int_from", 1},
            {u8"__fvalue_double_parse", 1},
            {u8"__fvalue_double_from", 1},
            {u8"__fvalue_string_from", 1},
            {u8"__ftime_now_ns", 0},
            /* math start */
            {u8"__fmath_acos", 1},
            {u8"__fmath_acosh", 1},
            {u8"__fmath_asin", 1},
            {u8"__fmath_asinh", 1},
            {u8"__fmath_atan", 1},
            {u8"__fmath_atan2", 2},
            {u8"__fmath_atanh", 1},
            {u8"__fmath_ceil", 1},
            {u8"__fmath_cos", 1},
            {u8"__fmath_cosh", 1},
            {u8"__fmath_exp", 1},
            {u8"__fmath_expm1", 1},
            {u8"__fmath_fabs", 1},
            {u8"__fmath_floor", 1},
            {u8"__fmath_fmod", 2},
            {u8"__fmath_frexp", 1},
            {u8"__fmath_gcd", 2},
            {u8"__fmath_hypot", 2},
            {u8"__fmath_isequal", 2},
            {u8"__fmath_log", 1},
            {u8"__fmath_log10", 1},
            {u8"__fmath_log1p", 1},
            {u8"__fmath_log2", 1},
            {u8"__fmath_sin", 1},
            {u8"__fmath_sinh", 1},
            {u8"__fmath_sqrt", 1},
            {u8"__fmath_tan", 1},
            {u8"__fmath_tanh", 1},
            {u8"__fmath_trunc", 1},
        };
        return builtinFunctionArgCounts;
    }
    const std::unordered_map<FString, BuiltinFunction> &getBuiltinFunctions()
    {
        static const std::unordered_map<FString, BuiltinFunction> builtinFunctions{
            {u8"__fstdout_print",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 for (auto arg : args) { std::print("{}", arg->toStringIO().toBasicString()); }
                 return std::make_shared<Object>(ValueType::IntClass(args.size()));
             }},
            {u8"__fstdout_println",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 for (auto arg : args) { std::print("{}", arg->toStringIO().toBasicString()); }
                 std::print("\n");
                 return std::make_shared<Object>(ValueType::IntClass(args.size()));
             }},
            {u8"__fstdin_read",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 std::string input;
                 std::cin >> input;
                 return std::make_shared<Object>(FString::fromBasicString(input));
             }},
            {u8"__fstdin_readln",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 std::string line;
                 std::getline(std::cin, line);
                 return std::make_shared<Object>(FString::fromBasicString(line));
             }},
            {u8"__fvalue_type",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 return std::make_shared<Object>(args[0]->getTypeInfo().toString());
             }},
            {u8"__fvalue_int_parse",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 FString str = args[0]->as<ValueType::StringClass>();
                 try
                 {
                     ValueType::IntClass val = std::stoi(str.toBasicString());
                     return std::make_shared<Object>(val);
                 }
                 catch (...)
                 {
                     throw RuntimeError(FString(std::format("Invalid int string for parsing", str.toBasicString())));
                 }
             }},
            {u8"__fvalue_int_from",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 if (val->is<ValueType::DoubleClass>())
                 {
                     return std::make_shared<Object>(
                         static_cast<ValueType::IntClass>(val->as<ValueType::DoubleClass>()));
                 }
                 else if (val->is<ValueType::BoolClass>())
                 {
                     return std::make_shared<Object>(
                         static_cast<ValueType::IntClass>(val->as<ValueType::BoolClass>() ? 1 : 0));
                 }
                 else
                 {
                     throw RuntimeError(FString(std::format("Type '{}' cannot be converted to int",
                                                            val->getTypeInfo().toString().toBasicString())));
                 }
             }},
            {u8"__fvalue_double_parse",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 FString str = args[0]->as<ValueType::StringClass>();
                 try
                 {
                     ValueType::DoubleClass val = std::stod(str.toBasicString());
                     return std::make_shared<Object>(ValueType::DoubleClass(val));
                 }
                 catch (...)
                 {
                     throw RuntimeError(FString(std::format("Invalid double string for parsing", str.toBasicString())));
                 }
             }},
            {u8"__fvalue_double_from",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 if (val->is<ValueType::IntClass>())
                 {
                     return std::make_shared<Object>(
                         static_cast<ValueType::DoubleClass>(val->as<ValueType::IntClass>()));
                 }
                 else if (val->is<ValueType::BoolClass>())
                 {
                     return std::make_shared<Object>(
                         ValueType::DoubleClass(val->as<ValueType::BoolClass>() ? 1.0 : 0.0));
                 }
                 else
                 {
                     throw RuntimeError(FString(std::format("Type '{}' cannot be converted to double",
                                                            val->getTypeInfo().toString().toBasicString())));
                 }
             }},
            {u8"__fvalue_string_from",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 return std::make_shared<Object>(val->toStringIO());
             }},
            {u8"__ftime_now_ns",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 // returns nanoseconds
                 using namespace Fig::Time;
                 auto now = Clock::now();
                 return std::make_shared<Object>(static_cast<ValueType::IntClass>(
                     std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time).count()));
             }},

            /* math start */
            {u8"__fmath_acos",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(acos(d));
             }},
            {u8"__fmath_acosh",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(acosh(d));
             }},
            {u8"__fmath_asin",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(asin(d));
             }},
            {u8"__fmath_asinh",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(asinh(d));
             }},
            {u8"__fmath_atan",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(atan(d));
             }},
            {u8"__fmath_atan2",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass y = args[0]->getNumericValue();
                 ValueType::DoubleClass x = args[1]->getNumericValue();
                 return std::make_shared<Object>(atan2(y, x));
             }},
            {u8"__fmath_atanh",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(atanh(d));
             }},
            {u8"__fmath_ceil",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(ceil(d));
             }},
            {u8"__fmath_cos",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(cos(d));
             }},
            {u8"__fmath_cosh",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(cosh(d));
             }},
            {u8"__fmath_exp",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(exp(d));
             }},
            {u8"__fmath_expm1",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(expm1(d));
             }},
            {u8"__fmath_fabs",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(fabs(d));
             }},
            {u8"__fmath_floor",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(floor(d));
             }},
            {u8"__fmath_fmod",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass x = args[0]->getNumericValue();
                 ValueType::DoubleClass y = args[1]->getNumericValue();
                 return std::make_shared<Object>(fmod(x, y));
             }},
            {u8"__fmath_frexp",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 int e;
                 return std::make_shared<Object>(List({std::make_shared<Object>(frexp(d, &e)),
                                                       std::make_shared<Object>(static_cast<ValueType::IntClass>(e))}));
             }},
            {u8"__fmath_gcd",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::IntClass x = args[0]->as<ValueType::IntClass>();
                 ValueType::IntClass y = args[1]->as<ValueType::IntClass>();
                 return std::make_shared<Object>(std::gcd(x, y));
             }},
            {u8"__fmath_hypot",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass x = args[0]->getNumericValue();
                 ValueType::DoubleClass y = args[1]->getNumericValue();
                 return std::make_shared<Object>(hypot(x, y));
             }},
            {u8"__fmath_isequal",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass x = args[0]->getNumericValue();
                 ValueType::DoubleClass y = args[1]->getNumericValue();
                 static const double epsilon = 1e-9;
                 return std::make_shared<Object>(fabs(x - y) < epsilon);
             }},
            {u8"__fmath_log",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log(d));
             }},
            {u8"__fmath_log10",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log10(d));
             }},
            {u8"__fmath_log1p",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log1p(d));
             }},
            {u8"__fmath_log2",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log2(d));
             }},
            {u8"__fmath_sin",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(sin(d));
             }},
            {u8"__fmath_sinh",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(sinh(d));
             }},
            {u8"__fmath_sqrt",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(sqrt(d));
             }},
            {u8"__fmath_tan",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(tan(d));
             }},
            {u8"__fmath_tanh",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(tanh(d));
             }},
            {u8"__fmath_trunc",
             [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(trunc(d));
             }},
        };
        return builtinFunctions;
    }
} // namespace Fig::Builtins