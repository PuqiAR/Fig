#pragma once

#include <IR/IR.hpp>

#include <cassert>

namespace Fig::IR
{
    struct VirtualMachine
    {
        std::vector<Function *> functions;

        int64_t execute(Function *fn, const int64_t *args) 
        {
            assert(fn != nullptr);
            std::vector<int64_t> regs(fn->regCount);
            // load params

            size_t ip = 0;

            while (ip < fn->code.size())
            {
                const Inst &ins = fn->code[ip++];

                switch (ins.op)
                {
                    case Op::Nop: break;

                    case Op::LoadImm: regs[ins.dst] = ins.imm; break;

                    case Op::Mov: regs[ins.dst] = regs[ins.a]; break;

                    case Op::Add: regs[ins.dst] = regs[ins.a] + regs[ins.b]; break;

                    case Op::Sub: regs[ins.dst] = regs[ins.a] - regs[ins.b]; break;

                    case Op::Mul: regs[ins.dst] = regs[ins.a] * regs[ins.b]; break;

                    case Op::Div: regs[ins.dst] = regs[ins.a] / regs[ins.b]; break;

                    case Op::Lt: regs[ins.dst] = regs[ins.a] < regs[ins.b]; break;

                    case Op::Le: regs[ins.dst] = regs[ins.a] <= regs[ins.b]; break;

                    case Op::Gt: regs[ins.dst] = regs[ins.a] > regs[ins.b]; break;

                    case Op::Ge: regs[ins.dst] = regs[ins.a] >= regs[ins.b]; break;

                    case Op::Eq: regs[ins.dst] = regs[ins.a] == regs[ins.b]; break;

                    case Op::Jmp: ip = static_cast<size_t>(static_cast<int64_t>(ip) + ins.imm); break;

                    case Op::Br:
                        if (regs[ins.a] == 0) { ip = static_cast<size_t>(static_cast<int64_t>(ip) + ins.imm); }
                        break;

                    case Op::Call: {
                        // currently supports 1-arg call via reg a
                        Function *callee = functions.at(static_cast<size_t>(ins.imm));
                        int64_t arg0 = regs[ins.a];
                        int64_t ret = execute(callee, &arg0);
                        regs[ins.dst] = ret;
                        break;
                    }

                    case Op::Ret: return regs[ins.a];
                }
            }

            // unreachable normally
            return 0;
        }
    };
}; // namespace Fig::IR