#pragma once

#include <Core/fig_string.hpp>
#include <Bytecode/Instruction.hpp>
#include <Evaluator/Value/value.hpp>

#include <vector>

namespace Fig
{
    struct ChunkAddressInfo
    {
        FString sourcePath;
        std::vector<FString> sourceLines;
    };

    struct Chunk
    {
        Instructions ins; // vector<Instruction>
        std::vector<Object> constants; // 常量池

        std::vector<InstructionAddressInfo> instructions_addr; // 下标和ins对齐，表示每个Instruction对应的地址
        ChunkAddressInfo addr; // 代码块独立Addr
    };
};