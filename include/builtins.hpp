#pragma once

#include <fig_string.hpp>
#include <value.hpp>

#include <unordered_map>
#include <functional>
#include <vector>
#include <print>
#include <iostream>

namespace Fig
{
    namespace Builtins
    {
        const std::unordered_map<FString, Object> builtinValues = {
            {u8"null", Object::getNullInstance()},
            {u8"true", Object(true)},
            {u8"false", Object(false)},
        };

        using BuiltinFunction = std::function<Object(const std::vector<Object> &)>;

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
        };

        const std::unordered_map<FString, BuiltinFunction> builtinFunctions{
            {u8"__fstdout_print", [](const std::vector<Object> &args) -> Object {
                 for (auto arg : args)
                 {
                     std::print("{}", arg.toString().toBasicString());
                 }
                 return Object(Int(args.size()));
             }},
            {u8"__fstdout_println", [](const std::vector<Object> &args) -> Object {
                 for (auto arg : args)
                 {
                     std::print("{}", arg.toString().toBasicString());
                 }
                 std::print("\n");
                 return Object(Int(args.size()));
             }},
            {u8"__fstdin_read", [](const std::vector<Object> &args) -> Object {
                 std::string input;
                 std::cin >> input;
                 return Object(FString::fromBasicString(input));
             }},
            {u8"__fstdin_readln", [](const std::vector<Object> &args) -> Object {
                 std::string line;
                 std::getline(std::cin, line);
                 return Object(FString::fromBasicString(line));
             }},
            {u8"__fvalue_type", [](const std::vector<Object> &args) -> Object {
                 return Object(args[0].getTypeInfo().toString());
             }},
            {u8"__fvalue_int_parse", [](const std::vector<Object> &args) -> Object {
                 FString str = args[0].as<String>().getValue();
                 try
                 {
                     ValueType::IntClass val = std::stoi(str.toBasicString());
                     return Object(Int(val));
                 }
                 catch (...)
                 {
                     throw RuntimeError(FStringView(std::format("Invalid int string for parsing", str.toBasicString())));
                 }
             }},
            {u8"__fvalue_int_from", [](const std::vector<Object> &args) -> Object {
                    Object val = args[0];
                    if (val.is<Double>())
                    {
                        return Object(Int(static_cast<ValueType::IntClass>(val.as<Double>().getValue())));
                    }
                    else if (val.is<Bool>())
                    {
                        return Object(Int(val.as<Bool>().getValue() ? 1 : 0));
                    }
                    else
                    {
                        throw RuntimeError(FStringView(std::format("Type '{}' cannot be converted to int", val.getTypeInfo().toString().toBasicString())));
                    }
             }},
            {u8"__fvalue_double_parse", [](const std::vector<Object> &args) -> Object {
                    FString str = args[0].as<String>().getValue();
                    try
                    {
                        ValueType::DoubleClass val = std::stod(str.toBasicString());
                        return Object(Double(val));
                    }
                    catch (...)
                    {
                        throw RuntimeError(FStringView(std::format("Invalid double string for parsing", str.toBasicString())));
                    }
             }},
            {u8"__fvalue_double_from", [](const std::vector<Object> &args) -> Object {
                    Object val = args[0];
                    if (val.is<Int>())
                    {
                        return Object(Double(static_cast<ValueType::DoubleClass>(val.as<Int>().getValue())));
                    }
                    else if (val.is<Bool>())
                    {
                        return Object(Double(val.as<Bool>().getValue() ? 1.0 : 0.0));
                    }
                    else
                    {
                        throw RuntimeError(FStringView(std::format("Type '{}' cannot be converted to double", val.getTypeInfo().toString().toBasicString())));
                    }
             }},
             {u8"__fvalue_string_from", [](const std::vector<Object> &args) -> Object {
                Object val = args[0];
                return Object(val.toString());
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
                throw RuntimeError(FStringView(std::format("Builtin function '{}' not found", name.toBasicString())));
            }
            return it->second;
        }

        inline int getBuiltinFunctionParamCount(const FString &name)
        {
            auto it = builtinFunctionArgCounts.find(name);
            if (it == builtinFunctionArgCounts.end())
            {
                throw RuntimeError(FStringView(std::format("Builtin function '{}' not found", name.toBasicString())));
            }
            return it->second;
        }
    }; // namespace Builtins
}; // namespace Fig