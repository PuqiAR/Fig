#pragma once

#include <Core/fig_string.hpp>

#include <vector>
#include <cstdint>
namespace Fig::IR
{
    using Reg = uint16_t;
    using Label = uint32_t;

    constexpr Reg INVALID_REG = 0xFFFF;

    enum class Op : uint8_t
    {
        // ---- control ----
        Nop,
        Jmp,
        Br, // conditional branch
        Ret,
        Call,

        // ---- arithmetic ----
        Add,
        Sub,
        Mul,
        Div,

        // ---- compare ----
        Lt,
        Le,
        Gt,
        Ge,
        Eq,

        // ---- data ----
        LoadImm, // immediate -> reg
        Mov,
    };

    struct Inst
    {
        Op op;

        Reg dst; // 结果寄存器
        Reg a;   // operand a
        Reg b;   // operand b

        int64_t imm; // immediate / jump offset
    };

    struct Function
    {
        FString name;

        uint16_t paramCount;
        uint16_t localCount; // 不含参数
        uint16_t regCount; // param + locals + temps

        std::vector<Inst> code;
    };
};