/*!
    @file src/Bytecode/Bytecode.hpp
    @brief 字节码Bytecode定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-18
*/

#pragma once

#include <cstdint>

#pragma once
#include <cstdint>

namespace Fig
{

    // 定长 32-bit
    using Instruction = std::uint32_t;

    enum class OpCode : std::uint8_t
    {
        Exit,      // 结束运行
        LoadK,     // iABx 模式: R[A] = Constants[Bx]
        LoadTrue,  // iABC: R[A] = true
        LoadFalse, // iABC: R[A] = false
        LoadNull,  // iABC: R[A] = null

        FastCall, // iABC: A: ProtoIdx, B: 函数起始寄存器
        Call,     // 动态派发 iABC: A: 函数体对象寄存器 B: 函数起始寄存器
        Return,   // iABC 模式: 返回 R[A] 的值

        LoadFn, // 惰性装修, iABx: R[A] = new FunctionObject...

        Jmp,        // iAsBx: ip += sBx 无条件跳转
        JmpIfFalse, // iAsBx: 如果 R[A] 为假, ip += sBx

        Mov,    // iABx: R[A] = R[Bx]
        Add,    // iABC: R[A] = R[B] + R[C]
        Sub,    // iABC: R[A] = R[B] - R[C]
        Mul,    // iABC: R[A] = R[B] * R[C]
        Div,    // iABC: R[A] = R[B] / R[C]
        Mod,    // iABC: R[A] = R[B] % R[C]
        BitXor, // iABC: R[A] = R[B] ^ R[C]

        Equal,        // iABC: R[A] = R[B] == R[C]
        NotEqual,     // iABC: R[A] = R[B] != R[C]
        Greater,      // iABC: R[A] = R[B] > R[C]
        Less,         // iABC: R[A] = R[B] < R[C]
        GreaterEqual, // iABC: R[A] = R[B] >= R[C]
        LessEqual,    // iABC: R[A] = R[B] <= R[C]

        Count, // 哨兵
    };

    namespace Op
    {
        // [OpCode: 8] [A: 8] [Bx: 16]
        [[nodiscard]] inline constexpr Instruction iABx(OpCode op, std::uint8_t a, std::uint16_t bx)
        {
            return static_cast<std::uint32_t>(op) | (static_cast<std::uint32_t>(a) << 8)
                   | (static_cast<std::uint32_t>(bx) << 16);
        }

        // [OpCode: 8] [A: 8] [B: 8] [C: 8]
        [[nodiscard]] inline constexpr Instruction iABC(
            OpCode op, std::uint8_t a, std::uint8_t b, std::uint8_t c)
        {
            return static_cast<std::uint32_t>(op) | (static_cast<std::uint32_t>(a) << 8)
                   | (static_cast<std::uint32_t>(b) << 16) | (static_cast<std::uint32_t>(c) << 24);
        }

        [[nodiscard]]
        inline constexpr Instruction iAsBx(OpCode op, std::uint8_t a, std::int16_t sbx)
        {
            return static_cast<std::uint32_t>(op) | (static_cast<std::uint32_t>(a) << 8)
                   | (static_cast<std::uint32_t>(static_cast<std::uint16_t>(sbx)) << 16);
        }
    } // namespace Op
} // namespace Fig