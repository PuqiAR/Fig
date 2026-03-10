/*!
    @file src/Sema/Environment.hpp
    @brief 树状符号表定义
*/

#pragma once

#include <Deps/Deps.hpp>
#include <Sema/Type.hpp>

namespace Fig
{
    enum class SymbolLocation
    {
        Global,
        Local,
        Upvalue
    };

    struct Symbol
    {
        String         name;
        Type           type;
        SymbolLocation location;
        int            index;
        bool           isConst;

        Symbol(String n, Type t, SymbolLocation l, int i, bool c) :
            name(std::move(n)), type(t), location(l), index(i), isConst(c)
        {
        }
    };

    struct UpvalueCapture
    {
        Symbol *target;
        int     index;
        bool    isLocal;
    };

    struct Scope
    {
        Scope *parent             = nullptr;
        bool   isFunctionBoundary = false;

        HashMap<String, Symbol *> locals;
        DynArray<UpvalueCapture>  upvalues;

        int nextLocalId = 0;

        Scope(Scope *p, bool isFn) : parent(p), isFunctionBoundary(isFn)
        {
            if (p && !isFn)
                nextLocalId = p->nextLocalId;
        }
    };

    class Environment
    {
    public:
        Scope *current = nullptr;

        void Push(bool isFn)
        {
            current = new Scope(current, isFn);
        }
        void Pop()
        {
            Scope *old = current;
            current    = current->parent;
            delete old;
        }
    };
} // namespace Fig
