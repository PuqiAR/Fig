/*!
    @file src/VM/VM.hpp
    @brief 虚拟机核心执行引擎
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <Compiler/Compiler.hpp>
#include <Object/Object.hpp>
#include <cassert>

#include <iostream> // debug
#include <print>

namespace Fig
{
    class VM
    {
    private:
        static constexpr unsigned int MAX_REGISTERS = 1024;

        // 一次性分配
        Value registers[MAX_REGISTERS];

    public:
        VM()
        {
            for (unsigned int i = 0; i < MAX_REGISTERS; ++i)
            {
                registers[i] = Value::GetNullInstance();
            }
        }

    private:
        inline OpCode decodeOpCode(Instruction inst)
        {
            return static_cast<OpCode>(inst & 0xFF);
        }
        inline std::uint8_t decodeA(Instruction inst)
        {
            return (inst >> 8) & 0xFF;
        }
        inline std::uint16_t decodeBx(Instruction inst)
        {
            return (inst >> 16) & 0xFFFF;
        }
        inline std::uint8_t decodeB(Instruction inst)
        {
            return (inst >> 16) & 0xFF;
        }
        inline std::uint8_t decodeC(Instruction inst)
        {
            return (inst >> 24) & 0xFF;
        }
        inline std::int16_t decodeSBx(Instruction inst)
        {
            return static_cast<std::int16_t>(inst >> 16);
        }

    public:
        // 执行入口：接收 Proto
        Result<Value, Error> Execute(Proto *proto);

        inline void PrintRegisters()
        {
            std::cout << "=== Registers ===" << '\n';
            for (unsigned int i = 0; i < MAX_REGISTERS; ++i)
            {
                Value &v = registers[i];
                if (!v.IsNull())
                {
                    std::println("[{}] {}", i, v.ToString());
                }
            }
        }
    };
} // namespace Fig