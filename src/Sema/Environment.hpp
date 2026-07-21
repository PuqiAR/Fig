/*!
    @file src/Sema/Environment.hpp
    @brief 符号表
    @author PuqiAR (im@puqiar.top)
    @date 2026-07-05
*/

#pragma once

#include <Deps/Deps.hpp>
#include <Sema/Type.hpp>

namespace Fig
{
    enum class SymbolKind : uint8_t
    {
        Var,
        Const,
        Func,
        Type,
    };

    struct Symbol
    {
        String     name;
        Type       type;
        SymbolKind kind;
        int        index;   // local: register idx, global: global idx, upvalue: upvalue idx

        bool isType() const { return kind == SymbolKind::Type; }
    };

    struct Scope
    {
        Scope                  *parent = nullptr;
        bool                    isFnBoundary = false;
        HashMap<String, Symbol *> locals;
        int                     nextReg = 0;
    };

    class Environment
    {
    public:
        Scope *current = nullptr;

        void push(bool isFn)
        {
            auto *s  = new Scope;
            s->parent = current;
            s->isFnBoundary = isFn;
            if (current && !isFn)
                s->nextReg = current->nextReg;
            current = s;
        }

        void pop()
        {
            auto *old = current;
            current   = current->parent;
            delete old;
        }
    };
} // namespace Fig
