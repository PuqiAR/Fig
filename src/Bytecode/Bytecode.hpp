/*!
    @file src/Bytecode/Bytecode.hpp
    @brief 字节码Bytecode定义
*/

#pragma once

#include <cstdint>

namespace Fig
{
    using Instruction = std::uint32_t;

    enum class OpCode : std::uint8_t
    {
        Exit,
        Exit_MaxRecursionDepthExceeded,

        LoadK,
        LoadTrue,
        LoadFalse,
        LoadNull,

        FastCall,
        Call,
        Return,

        LoadFn,

        Jmp,
        JmpIfFalse,

        Mov,

        Add,
        Sub,
        Mul,
        Div,
        Mod,
        BitXor,

        IntFastAdd,
        IntFastSub,
        IntFastMul,
        IntFastDiv,

        Equal,
        NotEqual,
        Greater,
        Less,
        GreaterEqual,
        LessEqual,

        GetGlobal,
        SetGlobal,
        GetUpval,
        SetUpval,
        Copy,

        Count
    };

    namespace Op
    {
        [[nodiscard]] inline constexpr Instruction iABx(OpCode op, std::uint8_t a, std::uint16_t bx)
        {
            return static_cast<std::uint32_t>(op) | (static_cast<std::uint32_t>(a) << 8)
                   | (static_cast<std::uint32_t>(bx) << 16);
        }

        [[nodiscard]] inline constexpr Instruction iABC(OpCode op, std::uint8_t a, std::uint8_t b, std::uint8_t c)
        {
            return static_cast<std::uint32_t>(op) | (static_cast<std::uint32_t>(a) << 8)
                   | (static_cast<std::uint32_t>(b) << 16) | (static_cast<std::uint32_t>(c) << 24);
        }

        [[nodiscard]] inline constexpr Instruction iAsBx(OpCode op, std::uint8_t a, std::int16_t sbx)
        {
            return static_cast<std::uint32_t>(op) | (static_cast<std::uint32_t>(a) << 8)
                   | (static_cast<std::uint32_t>(static_cast<std::uint16_t>(sbx)) << 16);
        }
    } // namespace Op
} // namespace Fig
