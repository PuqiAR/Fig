#pragma once

#include <unordered_map>
#include <iostream>
#include <algorithm>

#include <context_forward.hpp>
#include <fig_string.hpp>
#include <value.hpp>

namespace Fig
{
    struct Context
    {
    private:
        FString scopeName;
        std::unordered_map<FString, TypeInfo> varTypes;
        std::unordered_map<FString, Value> variables;
        std::unordered_map<FString, AccessModifier> ams;

    public:
        ContextPtr parent;

        Context(const Context &) = default;
        Context(const FString &name, ContextPtr p = nullptr) :
            scopeName(name), parent(p) {}
        Context(const FString &name, std::unordered_map<FString, TypeInfo> types, std::unordered_map<FString, Value> vars, std::unordered_map<FString, AccessModifier> _ams) :
            scopeName(std::move(name)), varTypes(std::move(types)), variables(std::move(vars)), ams(std::move(_ams)) {}

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

        std::optional<Value> get(const FString &name)
        {
            auto it = variables.find(name);
            if (it != variables.end())
                return it->second;
            if (parent)
                return parent->get(name);
            return std::nullopt;
        }
        AccessModifier getAccessModifier(const FString &name)
        {
            if (variables.contains(name))
            {
                return ams[name];
            }
            else if (parent != nullptr)
            {
                return parent->getAccessModifier(name);
            }
            else
            {
                throw RuntimeError(FStringView(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        ContextPtr createCopyWithPublicVariables()
        {
            std::unordered_map<FString, TypeInfo> _varTypes;
            std::unordered_map<FString, Value> _variables;
            std::unordered_map<FString, AccessModifier> _ams;
            for (const auto &p : this->variables)
            {
                if (isVariablePublic(p.first))
                {
                    _variables[p.first] = p.second;
                    _varTypes[p.first] = varTypes[p.first];
                    _ams[p.first] = ams[p.first];
                }
            }
            return std::make_shared<Context>(this->scopeName, _varTypes, _variables, _ams);
        }
        bool isVariableMutable(const FString &name)
        {
            AccessModifier am = getAccessModifier(name); // may throw
            return am == AccessModifier::Normal or am == AccessModifier::Public;
        }
        bool isVariablePublic(const FString &name)
        {
            AccessModifier am = getAccessModifier(name); // may throw
            return am == AccessModifier::Public or am == AccessModifier::PublicConst or am == AccessModifier::PublicFinal;
        }
        void set(const FString &name, const Value &value)
        {
            if (variables.contains(name))
            {
                if (!isVariableMutable(name))
                {
                    throw RuntimeError(FStringView(std::format("Variable '{}' is immutable", name.toBasicString())));
                }
                variables[name] = value;
            }
            else if (parent != nullptr)
            {
                parent->set(name, value);
            }
            else
            {
                throw RuntimeError(FStringView(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        void def(const FString &name, const TypeInfo &ti, AccessModifier am, const Value &value = Any())
        {
            if (variables.contains(name))
            {
                throw RuntimeError(FStringView(std::format("Variable '{}' already defined in this scope", name.toBasicString())));
            }
            variables[name] = value;
            varTypes[name] = ti;
            ams[name] = am;
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
        TypeInfo getTypeInfo(const FString &name)
        {
            if (varTypes.contains(name))
            {
                return varTypes[name];
            }
            else if (parent != nullptr)
            {
                return parent->getTypeInfo(name);
            }
            throw RuntimeError(FStringView(std::format("Variable '{}' not defined", name.toBasicString())));
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