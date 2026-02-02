#pragma once

#include <Ast/Expressions/VarExpr.hpp>

#include <Ast/functionParameters.hpp>
#include <Core/fig_string.hpp>
#include <Evaluator/Value/value.hpp>
#include <Core/runtimeTime.hpp>

#include <unordered_map>
#include <functional>
#include <vector>

namespace Fig
{
    namespace Builtins
    {

        /*
            // error's interface like:
            interface Error
            {
                toString() -> String;
                getErrorClass() -> String;
                getErrorMessage() -> String;
            }
        */

        inline static const TypeInfo &getErrorInterfaceTypeInfo()
        {
            static const TypeInfo ErrorInterfaceTypeInfo(u8"Error", true);
            return ErrorInterfaceTypeInfo;
        }

        /*
            interface Operation
            {
                add(..., ...) -> ...;
            }
        
        */

        inline static const TypeInfo &getOperationInterfaceTypeInfo() 
        {
            static const TypeInfo OperationInterfaceTypeInfo(u8"Operation", true);
            return OperationInterfaceTypeInfo;
        }

        const std::unordered_map<FString, ObjectPtr> &getBuiltinValues();

        using BuiltinFunction = std::function<ObjectPtr(const std::vector<ObjectPtr> &)>;

        const std::unordered_map<FString, int> &getBuiltinFunctionArgCounts();
        const std::unordered_map<FString, BuiltinFunction> &getBuiltinFunctions();

        inline bool isBuiltinFunction(const FString &name)
        {
            return getBuiltinFunctions().find(name) != getBuiltinFunctions().end();
        }

        inline BuiltinFunction getBuiltinFunction(const FString &name)
        {
            auto it = getBuiltinFunctions().find(name);
            if (it == getBuiltinFunctions().end())
            {
                throw RuntimeError(FString(std::format("Builtin function '{}' not found", name.toBasicString())));
            }
            return it->second;
        }

        inline int getBuiltinFunctionParamCount(const FString &name)
        {
            auto it = getBuiltinFunctionArgCounts().find(name);
            if (it == getBuiltinFunctionArgCounts().end())
            {
                throw RuntimeError(FString(std::format("Builtin function '{}' not found", name.toBasicString())));
            }
            return it->second;
        }
    }; // namespace Builtins
}; // namespace Fig.