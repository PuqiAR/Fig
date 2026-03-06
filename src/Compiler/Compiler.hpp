/*!
    @file src/Compiler/Compiler.hpp
    @brief 编译器定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <Ast/Ast.hpp>
#include <Bytecode/Bytecode.hpp>
#include <Deps/Deps.hpp>
#include <Error/Error.hpp>
#include <Object/Object.hpp>
#include <SourceManager/SourceManager.hpp>

#include <cassert>
#include <iostream>

namespace Fig
{
    // 编译产物-函数
    struct Proto
    {
        DynArray<Instruction> code;
        DynArray<Value>       constants;
        std::uint8_t          maxStack = 0; // 函数运行所需寄存器数量
    };

    struct LocalVar
    {
        int          localId; // AST 传来的纯数字 ID
        std::uint8_t reg;     // 分配到的物理寄存器 ID
        int          depth;   // 物理作用域深度(用于 EndScope 释放寄存器)
    };

    inline constexpr int MAX_LOCALS    = 250;
    inline constexpr int MAX_CONSTANTS = UINT16_MAX + 1;

    // 任何跨函数、跨模块的编译，都压入弹出这个 State
    struct FuncState
    {
        String     name;
        FuncState *enclosing = nullptr; // 指向外层状态 (支持闭包)
        Proto     *proto     = nullptr;

        std::uint8_t       freeReg    = 0;
        int                scopeDepth = 0;
        DynArray<LocalVar> locals;

        std::uint8_t fastRegMap[UINT8_MAX + 1]; // 256, 索引 = localId, 值 = 寄存器 id

        FuncState(String _name, FuncState *enc = nullptr) : name(std::move(_name)), enclosing(enc)
        {
            proto = new Proto();
            std::fill_n(fastRegMap, 256, UINT8_MAX); // 255代表未分配
        }
        // 注意：这里不 delete proto，因为 proto 是要作为编译产物吐出去的
    };

    struct CompiledModule
    {
        String            name;   // 供调试/打印
        DynArray<Proto *> protos; // 扁平化函数原型

        CompiledModule(String _name, DynArray<Proto *> _protos) :
            name(std::move(_name)), protos(std::move(_protos))
        {
        }

        ~CompiledModule()
        {
            for (auto *p : protos)
            {
                delete p;
            }
        }
    };

    class Compiler
    {
    private:
        String fileName;

        SourceManager &manager;
        FuncState     *current = nullptr; // 永远指向当前正在编译的上下文

        int mainFuncIndex = -1;
        HashMap<int, int> globalFuncMap; // localid -> ProtoIdx

    public:
        DynArray<Proto *> allProtos;

        struct FuncStateProtector
        {
            Compiler  *compiler;
            FuncState *prevState;

            [[nodiscard]]
            FuncStateProtector(Compiler *comp, FuncState *newState) :
                compiler(comp), prevState(comp->current)
            {
                compiler->current = newState;
            }

            ~FuncStateProtector()
            {
                compiler->current = prevState;
            }
        };

        Compiler(String _fileName, SourceManager &_manager) :
            fileName(std::move(_fileName)), manager(_manager)
        {
            // 初始化顶级作用域
            current = new FuncState("global", nullptr);
            allProtos.push_back(current->proto); // 最顶层, bootstrapper
        }

        ~Compiler()
        {
            // 内存清理 (如果有异常中断)
            while (current != nullptr)
            {
                FuncState *prev = current->enclosing;
                delete current;
                current = prev;
            }
        }

        Result<CompiledModule *, Error> Compile(Program *program);

    private:
        void PushState(String _name)
        {
            current = new FuncState(std::move(_name));
        }

        Proto *PopState()
        {
            FuncState *oldState      = current;
            Proto     *finishedProto = oldState->proto;

            current = oldState->enclosing;
            delete oldState;

            return finishedProto;
        }

        std::uint8_t AllocReg()
        {
            if (current->freeReg >= 255)
            {
                assert(false && "Register overflow!");
            }
            std::uint8_t reg = current->freeReg++;
            if (current->freeReg > current->proto->maxStack)
            {
                current->proto->maxStack = current->freeReg;
            }
            return reg;
        }

        void FreeReg(std::uint8_t reg)
        {
            // 如果这个寄存器被局部变量使用，不释放直接 Return
            for (const auto &local : current->locals)
            {
                if (local.reg == reg)
                {
                    return; // 拒绝释放，保护局部变量生命周期
                }
            }

            // 如果它是纯粹的临时计算结果），释放
            if (reg == current->freeReg - 1)
            {
                current->freeReg--;
            }
        }
        void Emit(Instruction inst)
        {
            current->proto->code.push_back(inst);
        }

        std::uint16_t AddConstant(Value v)
        {
            auto it =
                std::find(current->proto->constants.begin(), current->proto->constants.end(), v);
            if (it != current->proto->constants.end())
            {
                return std::distance(current->proto->constants.begin(), it);
            }

            current->proto->constants.push_back(v);
            return static_cast<std::uint16_t>(current->proto->constants.size() - 1);
        }

        void BeginScope()
        {
            current->scopeDepth++;
        }

        void EndScope()
        {
            current->scopeDepth--;
            while (!current->locals.empty() && current->locals.back().depth > current->scopeDepth)
            {
                FreeReg(current->locals.back().reg);
                current->locals.pop_back();
            }
        }

        std::uint8_t DeclareLocal(int localId)
        {
            std::uint8_t reg = AllocReg();
            current->locals.push_back(LocalVar{localId, reg, current->scopeDepth});
            current->fastRegMap[localId] = reg;
            return reg;
        }

        std::uint8_t DeclareLocal(int localId, std::uint8_t reg) // 表示复用哪个寄存器
        {
            current->locals.push_back(LocalVar{localId, reg, current->scopeDepth});
            current->fastRegMap[localId] = reg;
            return reg;
        }

        // 发射一条跳转指令，并返回它在代码数组里的绝对索引 (Index)
        int EmitJump(OpCode op, std::uint8_t aReg = 0)
        {
            // 预填 0
            Emit(Op::iAsBx(op, aReg, 0));
            return current->proto->code.size() - 1;
        }

        // 填真实偏移量到那条指令里
        void PatchJump(int instructionIndex)
        {
            // 目标地址就是当前代码数组的末尾
            int target = current->proto->code.size();

            // 相对偏移量 = 目标地址 - 指令自身所在的地址 - 1
            // (因为 VM 里的 ip 在取指后会自动 +1，所以偏移要减去 1)
            int offset = target - instructionIndex - 1;

            if (offset < INT16_MIN || offset > INT16_MAX)
            {
                assert(false && "PatchJump: Jump offset exceeds 16-bit signed limit!");
            }
            Instruction &inst = current->proto->code[instructionIndex];
            inst              = (inst & 0x0000FFFF)
                   | (static_cast<Instruction>(static_cast<std::uint16_t>(offset)) << 16);
        }

        SourceLocation makeSourceLocation(AstNode *node)
        {
            SourceLocation location = node->location; // copy
            location.functionName   = current->name;
            location.fileName       = fileName;
            return location;
        }

        Result<std::uint8_t, Error> compileIdentiExpr(IdentiExpr *);
        Result<std::uint8_t, Error> compileLiteral(LiteralExpr *);

        Result<std::uint8_t, Error> compileAssignment(
            InfixExpr *); // 编译赋值，由 CompileInfixExpr调用
        Result<std::uint8_t, Error> compileInfixExpr(InfixExpr *);

        Result<std::uint8_t, Error> compileLeftValue(
            Expr *); // 左值对象，可以是变量、结构体字段或模块对象

        Result<std::uint8_t, Error> compileCallExpr(CallExpr *);

        Result<std::uint8_t, Error> compileExpr(Expr *);

        /* Statements */
        Result<void, Error> compileVarDecl(VarDecl *);
        Result<void, Error> compileBlockStmt(BlockStmt *);
        Result<void, Error> compileIfStmt(IfStmt *);
        Result<void, Error> compileWhileStmt(WhileStmt *);
        Result<void, Error> compileFnDefStmt(FnDefStmt *);
        Result<void, Error> compileReturnStmt(ReturnStmt *);

        Result<void, Error> compileStmt(Stmt *);
    };

    inline void DisassembleInstruction(Instruction inst, std::size_t index)
    {
        // 提取OpCode (低 8 位)
        auto op = static_cast<OpCode>(inst & 0xFF);

        std::string_view opName = magic_enum::enum_name(op);

        // 所有指令至少都有 A 操作数 (8~15 位)
        std::uint8_t a = (inst >> 8) & 0xFF;

        // 地址补零，指令名左对齐占 10 字符
        std::cout << std::format("{:04d}  {:<10} ", index, opName);

        switch (op)
        {
            case OpCode::Exit: {
                break;
            }

            case OpCode::Mov: {
                // iABx 模式
                std::uint16_t bx = (inst >> 16) & 0xFFFF;
                std::cout << std::format("R{:<3} R[{}]", a, bx);
                break;
            }
            case OpCode::LoadK: {
                // iABx 模式：解析 Bx (16~31 位)
                std::uint16_t bx = (inst >> 16) & 0xFFFF;
                std::cout << std::format("R{:<3} K[{}]", a, bx);
                break;
            }

            case OpCode::Jmp:
            case OpCode::JmpIfFalse: {
                // iAsBx
                std::int16_t sbx = static_cast<std::uint16_t>(inst >> 16);
                std::cout << std::format("R{:<3} [{}]", a, sbx);
                break;
            }

            case OpCode::FastCall:
            {
                std::uint8_t b = (inst >> 16) & 0xFF;
                std::cout << std::format("Proto{:<3} R[{}]+", a, b);
                break;
            }
            case OpCode::Call:
            {
                std::uint8_t b = (inst >> 16) & 0xFF;
                std::cout << std::format("R{:<3} R[{}]+", a, b);
                break;
            }

            case OpCode::LoadTrue:
            case OpCode::LoadFalse:
            case OpCode::LoadNull:
            case OpCode::Add:
            case OpCode::Sub:
            case OpCode::Mul:
            case OpCode::Div:
            case OpCode::Mod: {
                // iABC 模式：解析 B (16~23 位) 和 C (24~31 位)
                std::uint8_t b = (inst >> 16) & 0xFF;
                std::uint8_t c = (inst >> 24) & 0xFF;
                std::cout << std::format("R{:<3} R{:<3} R{}", a, b, c);
                break;
            }
            case OpCode::Return: {
                // iA 模式：只用到了 A
                std::cout << std::format("R{}", a);
                break;
            }

            case OpCode::LoadFn: {
                std::uint16_t bx = (inst >> 16) & 0xFFFF;
                std::cout << std::format("R{:<3} Proto[{}]", a, bx);
                break;
            }

                // default: {
                //     std::cout << "?";
                //     break;
                // }
        }
        std::cout << '\n';
    }

    inline void DumpCode(const DynArray<Instruction> &code)
    {
        std::cout << "  Bytecode\n";
        for (std::size_t i = 0; i < code.size(); ++i)
        {
            DisassembleInstruction(code[i], i);
        }
    }
}; // namespace Fig