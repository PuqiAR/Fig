#pragma once

#include <Core/fig_string.hpp>
#include <Value/value.hpp>

#include <numeric>
#include <unordered_map>
#include <functional>
#include <vector>
#include <print>
#include <iostream>
#include <cmath>

namespace Fig
{
    namespace Builtins
    {
        const std::unordered_map<FString, ObjectPtr> builtinValues = {
            {u8"null", Object::getNullInstance()},
            {u8"true", Object::getTrueInstance()},
            {u8"false", Object::getFalseInstance()},
        };

        using BuiltinFunction = std::function<ObjectPtr(const std::vector<ObjectPtr> &)>;

        const std::unordered_map<FString, int> builtinFunctionArgCounts = {
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

        const std::unordered_map<FString, BuiltinFunction> builtinFunctions{
            {u8"__fstdout_print", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 for (auto arg : args)
                 {
                     std::print("{}", arg->toStringIO().toBasicString());
                 }
                 return std::make_shared<Object>(ValueType::IntClass(args.size()));
             }},
            {u8"__fstdout_println", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 for (auto arg : args)
                 {
                     std::print("{}", arg->toStringIO().toBasicString());
                 }
                 std::print("\n");
                 return std::make_shared<Object>(ValueType::IntClass(args.size()));
             }},
            {u8"__fstdin_read", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 std::string input;
                 std::cin >> input;
                 return std::make_shared<Object>(FString::fromBasicString(input));
             }},
            {u8"__fstdin_readln", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 std::string line;
                 std::getline(std::cin, line);
                 return std::make_shared<Object>(FString::fromBasicString(line));
             }},
            {u8"__fvalue_type", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 return std::make_shared<Object>(args[0]->getTypeInfo().toString());
             }},
            {u8"__fvalue_int_parse", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
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
            {u8"__fvalue_int_from", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 if (val->is<ValueType::DoubleClass>())
                 {
                     return std::make_shared<Object>(static_cast<ValueType::IntClass>(val->as<ValueType::DoubleClass>()));
                 }
                 else if (val->is<ValueType::BoolClass>())
                 {
                     return std::make_shared<Object>(static_cast<ValueType::IntClass>(val->as<ValueType::BoolClass>() ? 1 : 0));
                 }
                 else
                 {
                     throw RuntimeError(FString(std::format("Type '{}' cannot be converted to int", val->getTypeInfo().toString().toBasicString())));
                 }
             }},
            {u8"__fvalue_double_parse", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
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
            {u8"__fvalue_double_from", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 if (val->is<ValueType::IntClass>())
                 {
                     return std::make_shared<Object>(static_cast<ValueType::DoubleClass>(val->as<ValueType::IntClass>()));
                 }
                 else if (val->is<ValueType::BoolClass>())
                 {
                     return std::make_shared<Object>(ValueType::DoubleClass(val->as<ValueType::BoolClass>() ? 1.0 : 0.0));
                 }
                 else
                 {
                     throw RuntimeError(FString(std::format("Type '{}' cannot be converted to double", val->getTypeInfo().toString().toBasicString())));
                 }
             }},
            {u8"__fvalue_string_from", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 return std::make_shared<Object>(val->toString());
             }},
             /* math start */
            {u8"__fmath_acos", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(acos(d));
             }},
            {u8"__fmath_acosh", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(acosh(d));
             }},
            {u8"__fmath_asin", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(asin(d));
             }},
            {u8"__fmath_asinh", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(asinh(d));
             }},
            {u8"__fmath_atan", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(atan(d));
             }},
            {u8"__fmath_atan2", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass y = args[0]->getNumericValue();
                 ValueType::DoubleClass x = args[1]->getNumericValue();
                 return std::make_shared<Object>(atan2(y, x));
             }},
            {u8"__fmath_atanh", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(atanh(d));
             }},
            {u8"__fmath_ceil", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(ceil(d));
             }},
            {u8"__fmath_cos", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(cos(d));
             }},
            {u8"__fmath_cosh", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(cosh(d));
             }},
            {u8"__fmath_exp", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(exp(d));
             }},
            {u8"__fmath_expm1", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(expm1(d));
             }},
            {u8"__fmath_fabs", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(fabs(d));
             }},
            {u8"__fmath_floor", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(floor(d));
             }},
            {u8"__fmath_fmod", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass x = args[0]->getNumericValue();
                 ValueType::DoubleClass y = args[1]->getNumericValue();
                 return std::make_shared<Object>(fmod(x, y));
             }},
            {u8"__fmath_frexp", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 int e;
                 return std::make_shared<Object>(List(
                     {std::make_shared<Object>(frexp(d, &e)),
                      std::make_shared<Object>(static_cast<ValueType::IntClass>(e))}));
             }},
            {u8"__fmath_gcd", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::IntClass x = args[0]->as<ValueType::IntClass>();
                 ValueType::IntClass y = args[1]->as<ValueType::IntClass>();
                 return std::make_shared<Object>(std::gcd(x, y));
             }},
            {u8"__fmath_hypot", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass x = args[0]->getNumericValue();
                 ValueType::DoubleClass y = args[1]->getNumericValue();
                 return std::make_shared<Object>(hypot(x, y));
             }},
            {u8"__fmath_isequal", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ValueType::DoubleClass x = args[0]->getNumericValue();
                 ValueType::DoubleClass y = args[1]->getNumericValue();
                 static const double epsilon = 1e-9;
                 return std::make_shared<Object>(
                    fabs(x - y) < epsilon
                 );
             }},
            {u8"__fmath_log", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log(d));
             }},
            {u8"__fmath_log10", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log10(d));
             }},
            {u8"__fmath_log1p", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log1p(d));
             }},
            {u8"__fmath_log2", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(log2(d));
             }},
            {u8"__fmath_sin", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(sin(d));
             }},
            {u8"__fmath_sinh", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(sinh(d));
             }},
            {u8"__fmath_sqrt", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(sqrt(d));
             }},
            {u8"__fmath_tan", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(tan(d));
             }},
            {u8"__fmath_tanh", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(tanh(d));
             }},
            {u8"__fmath_trunc", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 ObjectPtr val = args[0];
                 ValueType::DoubleClass d = val->getNumericValue();
                 return std::make_shared<Object>(trunc(d));
             }},
        };

        inline bool isBuiltinFunction(const FString &name)
        {
            return builtinFunctions.find(name) != builtinFunctions.end();
        }

        inline BuiltinFunction getBuiltinFunction(const FString &name)
        {
            auto it = builtinFunctions.find(name);
            if (it == builtinFunctions.end())
            {
                throw RuntimeError(FString(std::format("Builtin function '{}' not found", name.toBasicString())));
            }
            return it->second;
        }

        inline int getBuiltinFunctionParamCount(const FString &name)
        {
            auto it = builtinFunctionArgCounts.find(name);
            if (it == builtinFunctionArgCounts.end())
            {
                throw RuntimeError(FString(std::format("Builtin function '{}' not found", name.toBasicString())));
            }
            return it->second;
        }
    }; // namespace Builtins
}; // namespace Fig.