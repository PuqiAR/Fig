#pragma once

/*

从树遍历到虚拟机!

*/

#include "Core/fig_string.hpp"
#include "Utils/magic_enum/magic_enum.hpp"
#include <cstdint>
#include <format>
#include <string_view>

namespace Fig
{
    using OpCodeType = uint8_t;
    enum class Bytecode : OpCodeType
    {
        HALT = 0x01, // 程序终止，后跟 8 位退出码

        POP = 0x10,

        LOAD_NULL = 0x20,
        LOAD_TRUE = 0x21,
        LOAD_FALSE = 0x22,
        LOAD_CON8 = 0x23,  // 跟 8  位索引 (0     - 255)
        LOAD_CON16 = 0x24, // 跟 16 位索引 (255   - 65535)
        LOAD_CON32 = 0x25, // 跟 32 位索引 (65536 - 2^32-1)

        ADD = 0x30, // +
        SUB = 0x31, // -
        MUL = 0x32, // *
        DIV = 0x33, // /
        MOD = 0x34, // %

        AND = 0x40, // &
        OR = 0x41,  // |
        XOR = 0x42, // ~
        NOT = 0x43, // ! (not)

        EQ = 0x50,   // ==
        GT = 0x51,   // >
        GTEQ = 0x52, // >=
        LT = 0x53,   // <
        LTEQ = 0x54, // <=

        JUMP16 = 0x60, // 跟 16 位索引 无条件
        JUMP32 = 0x61, // 跟 32 位索引 无条件

        JUMP16_IF_TRUE = 0x62, // 跟 16 位索引 栈顶为真跳转
        JUMP32_IF_TRUE = 0x63, // 跟 32 位索引 栈顶为真跳转

        JUMP16_IF_FALSE = 0x64, // 跟 16 位索引 栈顶为假跳转
        JUMP32_IF_FALSE = 0x65, // 跟 32 位索引 栈顶为假跳转

        LOAD_LOCAL16 = 0x70, // 后跟 16 位索引
        LOAD_LOCAL32 = 0x71, // 后跟 32 位索引
    };

    inline FString bytecode2string(Bytecode code)
    {
        const std::string_view &name = magic_enum::enum_name(code);
        return FString(FStringView(name));
    }

    inline FString reverseCompile(const std::vector<uint8_t> &src)
    {
        assert(src.size() >= 1);

        FString result;

        using enum Bytecode;
        for (size_t i = 0; i < src.size();)
        {
            Bytecode code = Bytecode(src[i]);
            switch (code)
            {
                case HALT: {
                    uint8_t quitCode = src[++i];
                    result += FString(std::format("HALT {}", static_cast<uint8_t>(quitCode))) + u8"\n";
                    break;
                }
                case LOAD_CON8: {
                    uint8_t id = src[++i];
                    result += FString(std::format("LOAD_CON8 {}", static_cast<uint8_t>(id))) + u8"\n";
                    break;
                }
                case LOAD_CON16:
                case JUMP16:
                case JUMP16_IF_TRUE:
                case JUMP16_IF_FALSE:
                case LOAD_LOCAL16: {
                    uint8_t high = src[++i];
                    uint8_t low = src[++i];
                    int32_t id = (high << 8) | low;
                    result += FString(std::format("{} {}", bytecode2string(code).toBasicString(), id))
                              + u8"\n";
                    break;
                }

                case LOAD_CON32:
                case JUMP32:
                case JUMP32_IF_TRUE:
                case JUMP32_IF_FALSE:
                case LOAD_LOCAL32: {
                    uint32_t b0 = static_cast<uint32_t>(src[++i]);
                    uint32_t b1 = static_cast<uint32_t>(src[++i]);
                    uint32_t b2 = static_cast<uint32_t>(src[++i]);
                    uint32_t b3 = static_cast<uint32_t>(src[++i]);

                    uint32_t id = (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
                    result += FString(std::format("{} {}", bytecode2string(code).toBasicString(), id))
                              + u8"\n";
                    break;
                }
                default: {
                    result += bytecode2string(code) + u8"\n";
                }
            }
            ++i;
        }
        return result;
    }
}; // namespace Fig