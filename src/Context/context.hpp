#pragma once

#include <unordered_map>
#include <iostream>
#include <memory>

#include <Context/context_forward.hpp>
#include <Core/fig_string.hpp>
#include <Value/value.hpp>
#include <Value/VariableSlot.hpp>

namespace Fig
{

    class Context : public std::enable_shared_from_this<Context>
    {
    private:
        FString scopeName;
        std::unordered_map<FString, std::shared_ptr<VariableSlot>> variables;

        std::unordered_map<std::size_t, Function> functions;
        std::unordered_map<std::size_t, FString> functionNames;
        std::unordered_map<std::size_t, FString> structTypeNames;

    public:
        ContextPtr parent;

        Context(const Context &) = default;
        Context(const FString &name, ContextPtr p = nullptr) :
            scopeName(name), parent(p) {}
        Context(const FString &name, std::unordered_map<FString, TypeInfo> types, std::unordered_map<FString, std::shared_ptr<VariableSlot>> vars, std::unordered_map<FString, AccessModifier> _ams) :
            scopeName(std::move(name)), variables(std::move(vars)) {}

        void setParent(ContextPtr _parent)
        {
            parent = _parent;
        }

        void setScopeName(FString _name)
        {
            scopeName = std::move(_name);
        }

        FString getScopeName() const
        {
            return scopeName;
        }

        void merge(const Context &c)
        {
            variables.insert(c.variables.begin(), c.variables.end());
            functions.insert(c.functions.begin(), c.functions.end());
            functionNames.insert(c.functionNames.begin(), c.functionNames.end());
            structTypeNames.insert(c.structTypeNames.begin(), c.structTypeNames.end());
        }

        std::unordered_map<size_t, Function> getFunctions() const
        {
            return functions;
        }

        std::shared_ptr<VariableSlot> get(const FString &name)
        {
            auto it = variables.find(name);
            if (it != variables.end())
                return it->second;
            if (parent)
                return parent->get(name);
            throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
        }
        AccessModifier getAccessModifier(const FString &name)
        {
            if (variables.contains(name))
            {
                return variables[name]->am;
            }
            else if (parent != nullptr)
            {
                return parent->getAccessModifier(name);
            }
            else
            {
                throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        bool isVariableMutable(const FString &name)
        {
            AccessModifier am = getAccessModifier(name); // may throw
            return !isAccessConst(am);
        }
        bool isVariablePublic(const FString &name)
        {
            AccessModifier am = getAccessModifier(name); // may throw
            return isAccessPublic(am);
        }
        void set(const FString &name, ObjectPtr value)
        {
            if (variables.contains(name))
            {
                if (!isVariableMutable(name))
                {
                    throw RuntimeError(FString(std::format("Variable '{}' is immutable", name.toBasicString())));
                }
                variables[name]->value = value;
            }
            else if (parent != nullptr)
            {
                parent->set(name, value);
            }
            else
            {
                throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        void _update(const FString &name, ObjectPtr value)
        {
            if (variables.contains(name))
            {
                variables[name]->value = value;
            }
            else if (parent != nullptr)
            {
                parent->set(name, value);
            }
            else
            {
                throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        void def(const FString &name, const TypeInfo &ti, AccessModifier am, const ObjectPtr &value = Object::getNullInstance())
        {
            if (containsInThisScope(name))
            {
                throw RuntimeError(FString(std::format("Variable '{}' already defined in this scope", name.toBasicString())));
            }
            variables[name] = std::make_shared<VariableSlot>(
                name,
                value,
                ti,
                am);
            if (ti == ValueType::Function and value->getTypeInfo() == ValueType::Function)
            {
                auto &fn = value->as<Function>();
                functions[fn.id] = fn;
                functionNames[fn.id] = name;
            }
            if (ti == ValueType::StructType)
            {
                auto &st = value->as<StructType>();
                structTypeNames[st.id] = name;
            }
        }
        std::optional<Function> getFunction(std::size_t id)
        {
            auto it = functions.find(id);
            if (it != functions.end())
            {
                return it->second;
            }
            else if (parent)
            {
                return parent->getFunction(id);
            }
            else
            {
                return std::nullopt;
            }
        }
        std::optional<FString> getFunctionName(std::size_t id)
        {
            auto it = functionNames.find(id);
            if (it != functionNames.end())
            {
                return it->second;
            }
            else if (parent)
            {
                return parent->getFunctionName(id);
            }
            else
            {
                return std::nullopt;
            }
        }
        std::optional<FString> getStructName(std::size_t id)
        {
            auto it = structTypeNames.find(id);
            if (it != structTypeNames.end())
            {
                return it->second;
            }
            else if (parent)
            {
                return parent->getFunctionName(id);
            }
            else
            {
                return std::nullopt;
            }
        }
        bool contains(const FString &name)
        {
            if (variables.contains(name))
            {
                return true;
            }
            else if (parent != nullptr)
            {
                return parent->contains(name);
            }
            return false;
        }
        bool containsInThisScope(const FString &name) const
        {
            return variables.contains(name);
        }

        TypeInfo getTypeInfo(const FString &name)
        {
            return get(name)->declaredType;
        }
        bool isInFunctionContext()
        {
            ContextPtr ctx = shared_from_this();
            while (ctx)
            {
                if (ctx->getScopeName().find(u8"<Function ") == 0)
                {
                    return true;
                }
                ctx = ctx->parent;
            }
            return false;
        }
        bool isInLoopContext()
        {
            ContextPtr ctx = shared_from_this();
            while (ctx)
            {
                if (ctx->getScopeName().find(u8"<While ") == 0 or ctx->getScopeName().find(u8"<For ") == 0)
                {
                    return true;
                }
                ctx = ctx->parent;
            }
            return false;
        }
        void printStackTrace(std::ostream &os = std::cerr, int indent = 0) const
        {
            const Context *ctx = this;
            std::vector<const Context *> chain;

            while (ctx)
            {
                chain.push_back(ctx);
                ctx = ctx->parent.get();
            }

            os << "[STACK TRACE]\n";
            for (int i = static_cast<int>(chain.size()) - 1; i >= 0; --i)
            {
                os << "  #" << (chain.size() - 1 - i)
                   << " " << chain[i]->scopeName.toBasicString()
                   << "\n";
            }
        }
    };
}; // namespace Fig