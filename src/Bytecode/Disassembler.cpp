/*!
    @file src/Bytecode/Disassembler.cpp
    @brief 字节码反汇编器实现
*/

#include <Bytecode/Disassembler.hpp>
#include <iostream>
#include <format>

namespace Fig
{
    void Disassembler::DisassembleModule(const CompiledModule *module)
    {
        if (!module) return;
        std::cout << "--- Module Disassembly ---" << std::endl;
        for (auto *proto : module->protos)
        {
            DisassembleProto(proto);
        }
    }

    void Disassembler::DisassembleProto(const Proto *proto)
    {
        if (!proto) return;

        std::cout << std::format("\n--- Proto: {} (Regs: {}, Params: {}) ---\n", 
                                 proto->name, proto->maxRegisters, proto->numParams);

        for (size_t i = 0; i < proto->code.size(); ++i)
        {
            Instruction inst = proto->code[i];
            OpCode      op   = static_cast<OpCode>(inst & 0xFF);
            uint8_t     a    = (inst >> 8) & 0xFF;
            
            std::cout << std::format("[{:04}] {:<12} ", i, magic_enum::enum_name(op));

            Format fmt = GetFormat(op);
            if (fmt == Format::ABC)
            {
                uint8_t b = (inst >> 16) & 0xFF;
                uint8_t c = (inst >> 24) & 0xFF;
                std::cout << std::format("A:{:<3} B:{:<3} C:{:<3}", a, b, c);
            }
            else if (fmt == Format::ABx)
            {
                uint16_t bx = (inst >> 16) & 0xFFFF;
                std::cout << std::format("A:{:<3} Bx:{:<5}", a, bx);
                
                // 自动关联常量池
                if (op == OpCode::LoadK && bx < proto->constants.size())
                {
                    std::cout << std::format(" ; {}", proto->constants[bx].ToString());
                }
            }
            else if (fmt == Format::AsBx)
            {
                int16_t sbx = static_cast<int16_t>((inst >> 16) & 0xFFFF);
                std::cout << std::format("A:{:<3} sBx:{:<5}", a, sbx);
                
                // 计算跳转绝对地址
                if (op == OpCode::Jmp || op == OpCode::JmpIfFalse)
                {
                    std::cout << std::format(" ; to [{:04}]", i + sbx + 1);
                }
            }
            std::cout << "\n";
        }

        if (!proto->constants.empty())
        {
            std::cout << "Constants:\n";
            for (size_t i = 0; i < proto->constants.size(); ++i)
            {
                std::cout << std::format("  [{}] {}\n", i, proto->constants[i].ToString());
            }
        }
    }

    Disassembler::Format Disassembler::GetFormat(OpCode op)
    {
        switch (op)
        {
            case OpCode::LoadK:
            case OpCode::Mov:
            case OpCode::GetGlobal:
            case OpCode::SetGlobal:
            case OpCode::LoadFn:
                return Format::ABx;

            case OpCode::Exit:
            case OpCode::Exit_MaxRecursionDepthExceeded:
            case OpCode::Jmp:
            case OpCode::JmpIfFalse:
                return Format::AsBx;

            default:
                return Format::ABC;
        }
    }
} // namespace Fig
