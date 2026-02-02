#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <memory>

#include <Ast/astBase.hpp>
#include <Ast/Statements/InterfaceDefSt.hpp>
#include <Evaluator/Value/function.hpp>
#include <Evaluator/Value/interface.hpp>
#include <Evaluator/Value/Type.hpp>
#include <Evaluator/Context/context_forward.hpp>
#include <Core/fig_string.hpp>
#include <Evaluator/Value/value.hpp>
#include <Evaluator/Value/VariableSlot.hpp>

namespace Fig
{

    struct ImplRecord
    {
        TypeInfo interfaceType;
        TypeInfo structType;

        std::unordered_map<FString, Function> implMethods;
    };

    struct OperationRecord
    {
        using UnaryOpFn = std::function<ObjectPtr(const ObjectPtr &)>;        
        using BinaryOpFn = std::function<ObjectPtr(const ObjectPtr &, const ObjectPtr &)>;

        std::unordered_map<Ast::Operator, UnaryOpFn> unOpRec;
        std::unordered_map<Ast::Operator, BinaryOpFn> binOpRec;

        bool hasUnaryOp(Ast::Operator op) const
        {
            return unOpRec.contains(op);
        }

        bool hasBinaryOp(Ast::Operator op) const
        {
            return binOpRec.contains(op);
        }

        const UnaryOpFn &getUnaryOpFn(Ast::Operator op) const
        {
            return unOpRec.at(op);
        }

        const BinaryOpFn &getBinaryOpFn(Ast::Operator op) const 
        {
            return binOpRec.at(op);
        }
    };

    class Context : public std::enable_shared_from_this<Context>
    {
    private:
        FString scopeName;
        std::unordered_map<FString, std::shared_ptr<VariableSlot>> variables;

        // std::unordered_map<std::size_t, Function> functions;
        // std::unordered_map<std::size_t, FString> functionNames;

        // implRegistry <Struct, ordered list of ImplRecord>
        std::unordered_map<TypeInfo, std::vector<ImplRecord>, TypeInfoHash> implRegistry;
        std::unordered_map<TypeInfo, OperationRecord, TypeInfoHash> opRegistry;
    public:
        ContextPtr parent;

        Context(const Context &) = default;
        Context(const FString &name, ContextPtr p = nullptr) : scopeName(name), parent(p) {}

        void setParent(ContextPtr _parent) { parent = _parent; }

        void setScopeName(FString _name) { scopeName = std::move(_name); }

        FString getScopeName() const { return scopeName; }

        void merge(const Context &c)
        {
            variables.insert(c.variables.begin(), c.variables.end());
            implRegistry.insert(c.implRegistry.begin(), c.implRegistry.end());
            opRegistry.insert(c.opRegistry.begin(), c.opRegistry.end());
            // structTypeNames.insert(c.structTypeNames.begin(),
            // c.structTypeNames.end());
        }

        std::unordered_map<size_t, Function> getFunctions() const
        {
            std::unordered_map<size_t, Function> result;
            for (auto &[name, slot] : variables)
            {
                if (slot->declaredType == ValueType::Function)
                {
                    const Function &fn = slot->value->as<Function>();
                    result[fn.id] = fn;
                }
            }
            return result;
        }

        std::shared_ptr<VariableSlot> get(const FString &name)
        {
            auto it = variables.find(name);
            if (it != variables.end()) return it->second;
            if (parent) return parent->get(name);
            throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
        }
        AccessModifier getAccessModifier(const FString &name)
        {
            if (variables.contains(name)) { return variables[name]->am; }
            else if (parent != nullptr) { return parent->getAccessModifier(name); }
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
            else if (parent != nullptr) { parent->set(name, value); }
            else
            {
                throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        void _update(const FString &name, ObjectPtr value)
        {
            if (variables.contains(name)) { variables[name]->value = value; }
            else if (parent != nullptr) { parent->_update(name, value); }
            else
            {
                throw RuntimeError(FString(std::format("Variable '{}' not defined", name.toBasicString())));
            }
        }
        void def(const FString &name,
                 const TypeInfo &ti,
                 AccessModifier am,
                 const ObjectPtr &value = Object::getNullInstance())
        {
            if (containsInThisScope(name))
            {
                throw RuntimeError(
                    FString(std::format("Variable '{}' already defined in this scope", name.toBasicString())));
            }
            variables[name] = std::make_shared<VariableSlot>(name, value, ti, am);
            // if (ti == ValueType::StructType)
            // {
            //     auto &st = value->as<StructType>();
            //     structTypeNames[st.id] = name;
            // }
        }
        void
        defReference(const FString &name, const TypeInfo &ti, AccessModifier am, std::shared_ptr<VariableSlot> target)
        {
            if (containsInThisScope(name))
            {
                throw RuntimeError(
                    FString(std::format("Variable '{}' already defined in this scope", name.toBasicString())));
            }
            variables[name] = std::make_shared<VariableSlot>(name, target->value, ti, am, true, target);
        }

        std::optional<FString> getFunctionName(std::size_t id)
        {
            for (auto &[name, slot] : variables)
            {
                if (slot->declaredType == ValueType::Function)
                {
                    const Function &fn = slot->value->as<Function>();
                    if (fn.id == id) { return name; }
                }
            }
            return std::nullopt;
        }
        // std::optional<FString> getStructName(std::size_t id)
        // {
        //     auto it = structTypeNames.find(id);
        //     if (it != structTypeNames.end())
        //     {
        //         return it->second;
        //     }
        //     else if (parent)
        //     {
        //         return parent->getStructName(id);
        //     }
        //     else
        //     {
        //         return std::nullopt;
        //     }
        // }
        bool contains(const FString &name)
        {
            if (variables.contains(name)) { return true; }
            else if (parent != nullptr) { return parent->contains(name); }
            return false;
        }
        bool containsInThisScope(const FString &name) const { return variables.contains(name); }

        TypeInfo getTypeInfo(const FString &name) { return get(name)->declaredType; }
        bool isInFunctionContext()
        {
            ContextPtr ctx = shared_from_this();
            while (ctx)
            {
                if (ctx->getScopeName().find(u8"<Function ") == 0) { return true; }
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

        bool hasImplRegisted(const TypeInfo &structType, const TypeInfo &interfaceType) const
        {
            auto it = implRegistry.find(structType);
            if (it != implRegistry.end())
            {
                for (auto &r : it->second)
                {
                    if (r.interfaceType == interfaceType) return true;
                }
            }
            return parent && parent->hasImplRegisted(structType, interfaceType);
        }

        std::unordered_map<TypeInfo, std::vector<ImplRecord>, TypeInfoHash> getImplRegistry() const
        {
            return implRegistry;
        }

        std::unordered_map<TypeInfo, std::vector<ImplRecord>, TypeInfoHash> &getImplRegistry() { return implRegistry; }

        std::optional<ImplRecord> getImplRecord(const TypeInfo &structType, const TypeInfo &interfaceType) const
        {
            auto it = implRegistry.find(structType);
            if (it != implRegistry.end())
            {
                for (auto &r : it->second)
                {
                    if (r.interfaceType == interfaceType) return r;
                }
            }

            if (parent) return parent->getImplRecord(structType, interfaceType);

            return std::nullopt;
        }

        void setImplRecord(const TypeInfo &structType, const TypeInfo &interfaceType, const ImplRecord &record)
        {
            auto &list = implRegistry[structType];

            for (auto &r : list)
            {
                if (r.interfaceType == interfaceType) return;
            }

            list.push_back(record); // order is the level
        }

        bool hasMethodImplemented(const TypeInfo &structType, const FString &functionName) const
        {
            auto it = implRegistry.find(structType);
            if (it != implRegistry.end())
            {
                for (auto &record : it->second)
                {
                    if (record.implMethods.contains(functionName)) return true;
                }
            }

            return parent && parent->hasMethodImplemented(structType, functionName);
        }

        bool hasDefaultImplementedMethod(const TypeInfo &structType, const FString &functionName) const
        {
            auto it = implRegistry.find(structType);
            if (it == implRegistry.end()) return false;

            std::vector<TypeInfo> implementedInterfaces;
            for (auto &record : it->second) implementedInterfaces.push_back(record.interfaceType);

            for (auto &[_, slot] : variables)
            {
                if (!slot->value->is<InterfaceType>()) continue;

                InterfaceType &interface = slot->value->as<InterfaceType>();

                bool implemented = std::any_of(implementedInterfaces.begin(),
                                               implementedInterfaces.end(),
                                               [&](const TypeInfo &ti) { return ti == interface.type; });

                if (!implemented) continue;

                for (auto &method : interface.methods)
                {
                    if (method.name == functionName && method.hasDefaultBody()) return true;
                }
            }

            return false;
        }

        Ast::InterfaceMethod getDefaultImplementedMethod(const TypeInfo &structType, const FString &functionName)
        {
            // O(N²)
            // SLOW
            // wwww (sad)

            // huh
            // just let it be SLOW
            // i dont give a fuck

            auto it = implRegistry.find(structType);
            if (it == implRegistry.end()) assert(false);

            std::vector<TypeInfo> implementedInterfaces;
            for (auto &record : it->second) implementedInterfaces.push_back(record.interfaceType);

            for (auto &[_, slot] : variables)
            {
                if (!slot->value->is<InterfaceType>()) continue;

                InterfaceType &interface = slot->value->as<InterfaceType>();

                bool implemented = std::any_of(implementedInterfaces.begin(),
                                               implementedInterfaces.end(),
                                               [&](const TypeInfo &ti) { return ti == interface.type; });

                if (!implemented) continue;

                for (auto &method : interface.methods)
                {
                    if (method.name == functionName)
                    {
                        if (!method.hasDefaultBody()) assert(false);
                        return method;
                    }
                }
            }

            assert(false);
        }

        const Function &getImplementedMethod(const TypeInfo &structType, const FString &functionName) const
        {
            auto it = implRegistry.find(structType);
            if (it != implRegistry.end())
            {
                for (auto &record : it->second)
                {
                    auto fit = record.implMethods.find(functionName);
                    if (fit != record.implMethods.end()) return fit->second;
                }
            }

            if (parent) return parent->getImplementedMethod(structType, functionName);

            assert(false); // not found
            throw "";      // ignore warning
        }

        bool hasOperatorImplemented(const TypeInfo &type, Ast::Operator op, bool isUnary = false) const
        {
            auto it = opRegistry.find(type);
            if (it != opRegistry.end())
            {
                const OperationRecord &rec = it->second;
                if (isUnary)
                    return rec.hasUnaryOp(op);
                else
                    return rec.hasBinaryOp(op);
            }

            if (parent) return parent->hasOperatorImplemented(type, op, isUnary);
            return false;
        }

        std::optional<OperationRecord::UnaryOpFn> getUnaryOperatorFn(const TypeInfo &type, Ast::Operator op) const
        {
            auto it = opRegistry.find(type);
            if (it != opRegistry.end())
            {
                const OperationRecord &rec = it->second;
                if (rec.hasUnaryOp(op)) return rec.getUnaryOpFn(op);
            }

            if (parent) return parent->getUnaryOperatorFn(type, op);
            return std::nullopt;
        }

        std::optional<OperationRecord::BinaryOpFn> getBinaryOperatorFn(const TypeInfo &type, Ast::Operator op) const
        {
            auto it = opRegistry.find(type);
            if (it != opRegistry.end())
            {
                const OperationRecord &rec = it->second;
                if (rec.hasBinaryOp(op)) return rec.getBinaryOpFn(op);
            }

            if (parent) return parent->getBinaryOperatorFn(type, op);
            return std::nullopt;
        }

        void registerUnaryOperator(const TypeInfo &type, Ast::Operator op, OperationRecord::UnaryOpFn fn)
        {
            opRegistry[type].unOpRec[op] = std::move(fn);
        }

        void registerBinaryOperator(const TypeInfo &type, Ast::Operator op, OperationRecord::BinaryOpFn fn)
        {
            opRegistry[type].binOpRec[op] = std::move(fn);
        }

        bool removeUnaryOperator(const TypeInfo &type, Ast::Operator op)
        {
            auto it = opRegistry.find(type);
            if (it != opRegistry.end()) return it->second.unOpRec.erase(op) > 0;
            return false;
        }

        bool removeBinaryOperator(const TypeInfo &type, Ast::Operator op)
        {
            auto it = opRegistry.find(type);
            if (it != opRegistry.end()) return it->second.binOpRec.erase(op) > 0;
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
                os << "  #" << (chain.size() - 1 - i) << " " << chain[i]->scopeName.toBasicString() << "\n";
            }
        }
    };
}; // namespace Fig