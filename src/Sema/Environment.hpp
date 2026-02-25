/*!
    @file src/Sema/Environment.hpp
    @brief 符号和作用域环境定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-23
*/

#pragma once

#include <Deps/Deps.hpp>
#include <Error/Error.hpp>
#include <Sema/Type.hpp>

#include <cassert>
#include <optional>

namespace Fig
{
    // 记录在 Analyzer 中的符号元数据
    struct Symbol
    {
        String  name;
        TypeTag type;
        bool    isPublic;
        int     depth;      // 词法作用域深度
        bool    isConstant; // 是否是 const 声明的不可变常量 (用于报错: 尝试修改常量)

        int localId = -1; // Analyzer 虚拟槽位分配
    };

    // 作用域回档水位线
    struct ScopeWatermark
    {
        std::size_t symbolCount;
        int         savedLocalId;
    };

    // 语义分析函数上下文 (隔离局部变量 ID 空间)
    struct SemaFuncState
    {
        SemaFuncState           *enclosing    = nullptr;
        int                      currentDepth = 0;
        int                      nextLocalId  = 0;
        DynArray<ScopeWatermark> scopeStack;
    };

    class Environment
    {
    private:
        DynArray<Symbol> symbols;
        SemaFuncState   *current = nullptr;

    public:
        Environment()
        {
            current = new SemaFuncState();
        }

        ~Environment()
        {
            while (current)
            {
                SemaFuncState *prev = current->enclosing;
                delete current;
                current = prev;
            }
        }

        // 函数边界控

        void EnterFunction()
        {
            SemaFuncState *newState = new SemaFuncState();
            newState->enclosing     = current;
            current                 = newState;
        }

        void LeaveFunction()
        {
            assert(current && "Environment: Unmatched LeaveFunction");
            SemaFuncState *oldState = current;
            current                 = oldState->enclosing;
            delete oldState;
        }

        // 词法作用域控制

        void EnterScope()
        {
            current->currentDepth++;
            current->scopeStack.push_back({symbols.size(), current->nextLocalId});
        }

        void LeaveScope()
        {
            assert(current->currentDepth > 0 && "Environment: Unmatched LeaveScope");
            current->currentDepth--;

            assert(!current->scopeStack.empty());
            ScopeWatermark archive = current->scopeStack.back();
            current->scopeStack.pop_back();

            // 物理截断符号表，回滚槽位发号器以复用物理寄存器
            while (symbols.size() > archive.symbolCount)
            {
                symbols.pop_back();
            }
            current->nextLocalId = archive.savedLocalId;
        }

        // 符号操作

        // 注册符号, 返回分配的 localId。调用前内部执行同级作用域重定义断言
        int Define(const String &name, TypeTag type, bool isPublic, bool isConst)
        {
            for (auto it = symbols.rbegin(); it != symbols.rend(); ++it)
            {
                if (it->depth < current->currentDepth)
                    break;
                if (it->name == name)
                {
                    assert(false && "Environment.Define: redefinition");
                }
            }

            int allocatedId = current->nextLocalId++;
            symbols.push_back({name, type, isPublic, current->currentDepth, isConst, allocatedId});
            return allocatedId;
        }

        // 解析符号。找不到返回 nullopt
        std::optional<Symbol> Resolve(const String &name)
        {
            for (auto it = symbols.rbegin(); it != symbols.rend(); ++it)
            {
                if (it->name == name)
                    return *it;
            }
            return std::nullopt;
        }

        int GetDepth() const
        {
            return current->currentDepth;
        }
    };
} // namespace Fig