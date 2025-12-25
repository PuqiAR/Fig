#pragma once

#include <Core/fig_string.hpp>
#include <Value/value.hpp>

#include <unordered_map>
#include <functional>
#include <vector>
#include <print>
#include <iostream>

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
        };

        const std::unordered_map<FString, BuiltinFunction> builtinFunctions{
            {u8"__fstdout_print", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 bool first_flag = true;
                 for (auto arg : args)
                 {
                     if (!first_flag)
                         std::print(" ");
                     std::print("{}", arg->toStringIO().toBasicString());
                     first_flag = false;
                 }
                 return std::make_shared<Object>(ValueType::IntClass(args.size()));
             }},
            {u8"__fstdout_println", [](const std::vector<ObjectPtr> &args) -> ObjectPtr {
                 bool first_flag = true;
                 for (auto arg : args)
                 {
                     if (!first_flag)
                         std::print(" ");
                     std::print("{}", arg->toStringIO().toBasicString());
                     first_flag = false;
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
}; // namespace Fig