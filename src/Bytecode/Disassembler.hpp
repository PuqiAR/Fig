/*!
    @file src/Bytecode/Disassembler.hpp
    @brief 字节码反汇编器：物理还原指令语义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <Bytecode/Bytecode.hpp>
#include <Compiler/Compiler.hpp>

namespace Fig
{
    class Disassembler
    {
    public:
        // 反汇编整个模块
        static void DisassembleModule(const CompiledModule *module);

        // 反汇编单个函数原型
        static void DisassembleProto(const Proto *proto);

    private:
        enum class Format
        {
            ABC,
            ABx,
            AsBx
        };

        static Format GetFormat(OpCode op);
    };
} // namespace Fig
