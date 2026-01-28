#pragma once

#include <cstdint>
#include <cassert>
#include <vector>

namespace Fig
{
    using u8 = uint8_t;
    enum class OpCode : u8
    {
        HALT = 0, // 程序结束
        RETURN,

        LOAD_LOCAL,
        LOAD_CONST,

        STORE_LOCAL,

        LT,
        LTET,
        GT,
        GTET,
        ADD,
        SUB,
        MUL,
        DIV,

        JUMP,          // + 64 offset (int64_t)
        JUMP_IF_FALSE, // + 64 offset (int64_t)

        CALL,
    };

    static constexpr int MAX_LOCAL_COUNT = UINT64_MAX;
    static constexpr int MAX_CONSTANT_COUNT = UINT64_MAX;
    static constexpr int MAX_FUNCTION_ARG_COUNT = UINT64_MAX;

    inline OpCode getLastOpCode()
    {
        return OpCode::RETURN;
    }

    struct InstructionAddressInfo
    {
        size_t line, column;
    };

    using ProgramCounter = uint64_t;
    using InstructionPoint = uint64_t;

    class Instruction
    {
    public:
        OpCode code;

        int64_t operand;

        Instruction(OpCode _code) : code(_code) {}

        Instruction(OpCode _code, int64_t _operand)
        {
            code = _code;
            operand = _operand;
        }
    };

    using Instructions = std::vector<Instruction>;
}; // namespace Fig