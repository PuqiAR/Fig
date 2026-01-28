#pragma once

#include <Bytecode/Instruction.hpp>
#include <Bytecode/Chunk.hpp>

namespace Fig
{
    struct CompiledFunction
    {

        Chunk chunk; // 函数代码块

        FString name;  // 函数名
        uint64_t posArgCount; // 位置参数数量
        uint64_t defArgCount; // 默认参数数量
        bool variadicPara;    // 可变参数（是：最后为可变，否：不可变）

        uint64_t localCount;  // 局部变量数量(不包括参数)
        uint64_t slotCount; // = 总参数数量 + 局部变量数量
    };
};