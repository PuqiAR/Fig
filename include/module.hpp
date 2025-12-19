#pragma once

#include <memory>

#include <fig_string.hpp>
#include <value.hpp>
#include <context.hpp>

namespace Fig
{
    class Module
    {
    public:
        const FString name;
        const FString spec;
        const FString path;
        
        std::shared_ptr<Context> context; // module-level context

        /*
        
        import module -> automatically create a module context and call function `init` if exists
        all global functions, variables, structs, etc will be stored in module context
        then module context will be linked to the current context
        
        */
        Module(const FString &moduleName, const FString &moduleSpec, const FString &modulePath) :
            name(moduleName), spec(moduleSpec), path(modulePath)
        {
            context = std::make_shared<Context>(FString(std::format("<Module {}>", name.toBasicString())), nullptr);
        }

        bool hasSymbol(const FString &symbolName)
        {
            return context->contains(symbolName);
        }
        Value getSymbol(const FString &symbolName)
        {
            auto valOpt = context->get(symbolName);
            if (!valOpt.has_value())
            {
                throw RuntimeError(FStringView(std::format("Symbol '{}' not found in module '{}'", symbolName.toBasicString(), name.toBasicString())));
            }
            return valOpt.value();
        }
    };
};