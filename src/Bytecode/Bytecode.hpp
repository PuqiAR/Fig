/*!
    @file src/Bytecode/Bytecode.hpp
    @brief 字节码Bytecode定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-17
*/

#pragma once

#include <cstdint>

namespace Fig
{
    using OpCodeType = uint8_t;

    enum class OpCode : OpCodeType
    {
        LoadConst,  // dst, const id
        LoadLocal,  // dst, slot id
        StoreLocal, // slot, src(reg)

        LoadLocalRef, // dst, slot
        LoadRef,      // dst, refReg
        StoreRef,     // refReg, srcReg

        Add, // dst, a, b
        Move, // dst, src
    };
}; // namespace Fig