#pragma once

#include <Bytecode/CompiledFunction.hpp>
#include <Bytecode/Instruction.hpp>
#include <Bytecode/Chunk.hpp>

namespace Fig
{
    struct CallFrame
    {
        uint64_t ip;         // 函数第一个指令 index
        uint64_t base;       // 第一个参数在栈中位置偏移量

        CompiledFunction fn; // 编译过的函数体
    };

};