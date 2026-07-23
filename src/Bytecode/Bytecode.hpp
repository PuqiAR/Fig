/*!
    @file src/Bytecode/Bytecode.hpp
    @brief 字节码Bytecode定义
*/

#pragma once

#include <Deps/Deps.hpp>
#include <Object/ObjectBase.hpp>
#include <Core/SourceLocations.hpp>

#include <cstdint>


namespace Fig
{
    using Instruction = std::uint32_t;

    enum class OpCode : std::uint8_t
    {
        // 控制流
        Exit,                          // iAsBx, return sBx
        Exit_MaxRecursionDepthExceeded, // iAsBx, fatal: 超出最大递归深度

        // 常量加载
        LoadK,                         // iABx, R(A) = K(Bx)
        LoadTrue,                      // iABC, R(A) = true
        LoadFalse,                     // iABC, R(A) = false
        LoadNull,                      // iABC, R(A) = null

        // 函数调用
        FastCall,                      // iABC, call Proto[A], args from R(B)
        Call,                          // iABC, call R(A), args from R(B)
        Return,                        // iABC, return R(A)

        // 闭包
        LoadFn,                        // iABx, R(A) = new Closure(Proto[Bx])

        // 跳转
        Jmp,                           // iAsBx, PC += sBx
        JmpIfFalse,                    // iAsBx, if !R(A) then PC += sBx

        // 寄存器移动
        Mov,                           // iABx, R(A) = R(Bx)

        // 算术运算
        Add,                           // iABC, R(A) = R(B) + R(C)
        Sub,                           // iABC, R(A) = R(B) - R(C)
        Mul,                           // iABC, R(A) = R(B) * R(C)
        Div,                           // iABC, R(A) = R(B) / R(C)
        Mod,                           // iABC, R(A) = R(B) % R(C)   (WIP)
        BitXor,                        // iABC, R(A) = R(B) ^ R(C)   (WIP)

        // 快速整数算术（仅 int，无类型检查）
        IntFastAdd,                    // iABC, R(A) = R(B) + R(C)  (int)
        IntFastSub,                    // iABC, R(A) = R(B) - R(C)  (int)
        IntFastMul,                    // iABC, R(A) = R(B) * R(C)  (int)
        // 结果可能为非整数
        IntFastDiv,                    // iABC, R(A) = (double)R(B) / R(C)  (int)

        // 比较
        Equal,                         // iABC, R(A) = R(B) == R(C)
        NotEqual,                      // iABC, R(A) = R(B) != R(C)
        Greater,                       // iABC, R(A) = R(B) >  R(C)
        Less,                          // iABC, R(A) = R(B) <  R(C)
        GreaterEqual,                  // iABC, R(A) = R(B) >= R(C)
        LessEqual,                     // iABC, R(A) = R(B) <= R(C)

        // 变量存取
        GetGlobal,                     // iABx, R(A) = G(Bx)
        SetGlobal,                     // iABx, G(Bx) = R(A)
        GetUpval,                      // iABC, R(A) = *Upval(B)
        SetUpval,                      // iABC, *Upval(B) = R(A)
        Copy,                          // iABC, R(A) = R(B)

        Count                          // 哨兵
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

    struct UpvalueInfo
    {
        uint8_t index;
        bool    isLocal;
    };

    struct Proto
    {
        String                name;
        DynArray<Instruction> code;
        DynArray<SourceLocation *> locations;
        DynArray<Value>       constants;
        DynArray<UpvalueInfo> upvalues;
        uint8_t               maxRegisters = 0;
        uint8_t               numParams    = 0;
    };

    struct CompiledModule
    {
        DynArray<Proto *> protos;
    };

} // namespace Fig
