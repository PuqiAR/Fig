/*!
    @file src/Sema/Environment.hpp
    @brief 符号和作用域环境定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-23
*/

#pragma once

#include <Sema/Type.hpp>
#include <Deps/Deps.hpp>
#include <Error/Error.hpp>

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
    };

    class Environment
    {
    private:
        DynArray<Symbol> symbols;
        int              currentDepth = 0;

    public:
        void EnterScope()
        {
            currentDepth++;
        }
        void LeaveScope()
        {
            while (!symbols.empty() && symbols.back().depth > currentDepth)
            {
                symbols.pop_back();
            }
            currentDepth--;
        }

        // 注册符号, 调用前确保无重复 (内部assert)
        void Define(const String &name, TypeTag type, bool isPublic, bool isConst)
        {
            for (auto it = symbols.rbegin(); it != symbols.rend(); ++it)
            {
                if (it->depth < currentDepth)
                    break;
                if (it->name == name)
                {
                    assert(false && "Environment.Define: redefinition");
                }
            }
            symbols.push_back({name, type, isPublic, currentDepth, isConst});
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
            return currentDepth;
        }
    };
} // namespace Fig