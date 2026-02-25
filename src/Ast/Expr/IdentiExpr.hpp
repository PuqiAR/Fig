/*!
    @file src/Ast/Expr/IdentiExpr.hpp
    @brief IdentiExpr定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Ast/Base.hpp>
#include <Deps/Deps.hpp>

namespace Fig
{
    struct IdentiExpr final : Expr 
    {
        String name;
        
        // Analyzer槽位

        // 寻址空间
        bool isGlobal = false; // 是否全局, 全局变量存哈希/堆

        // 仅 isGlobal = false 有效
        int resolvedDepth = -1; // 用于获取闭包上值, -1 代表未解析

        // 栈内槽位
        int localId = -1; // 局部变量id, -1 未解析


        IdentiExpr()
        {
            type = AstType::IdentiExpr;
        }

        IdentiExpr(String _name, SourceLocation _loc)
        {
            type = AstType::IdentiExpr;
            name = std::move(_name);
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            return std::format("<IdentiExpr: {}>", name);
        }
    };
};