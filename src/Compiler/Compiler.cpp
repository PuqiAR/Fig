/*!
    @file src/Compiler/Compiler.cpp
    @brief 编译器主逻辑实现：物理 Bootstrapper 与双步扫描
*/

#include <Ast/Stmt/FnDefStmt.hpp>
#include <Compiler/Compiler.hpp>

namespace Fig
{
    Result<CompiledModule *, Error> Compiler::Compile(Program *program)
    {
        module = new CompiledModule();
        if (program->nodes.empty())
        {
            return module;
        }

        // 预留 Protos[0] 给 Bootstrapper
        Proto *bootProto = new Proto();
        bootProto->name  = "[bootstrapper]";
        module->protos.push_back(bootProto);

        int initIdx = -1;
        int mainIdx = -1;

        SourceLocation *mainFnLoc = nullptr;
        SourceLocation *initFnLoc = nullptr;

        // 预扫描顶层函数
        for (auto *stmt : program->nodes)
        {
            if (stmt->type == AstType::FnDefStmt)
            {
                auto  *f        = static_cast<FnDefStmt *>(stmt);
                int    idx      = (int) module->protos.size();

                Proto *p        = new Proto();
                p->name         = f->name;
                p->numParams    = (uint8_t) f->params.size();
                p->maxRegisters = p->numParams;
                f->protoIndex = idx;

                module->protos.push_back(p);

                // 连接物理符号到索引
                if (f->resolvedSymbol)
                {
                    f->resolvedSymbol->index = idx;
                }

                if (f->name == "init")
                {
                    initIdx   = idx;
                    initFnLoc = &stmt->location;
                }
                if (f->name == "main")
                {
                    mainIdx   = idx;
                    mainFnLoc = &stmt->location;
                }
            }
        }

        // Bootstrapper 中编译所有语句
        FuncState bootState(bootProto, nullptr);
        current = &bootState;

        for (auto *stmt : program->nodes)
        {
            auto res = compileStmt(stmt);
            if (!res)
            {
                return std::unexpected(res.error());
            }
        }

        // 发射 Bootstrapper 引导指令
        if (initIdx != -1)
        {
            emit(Op::iABC(OpCode::FastCall, (uint8_t) initIdx, 0, 0), initFnLoc);
        }

        if (mainIdx != -1)
        {
            emit(Op::iABC(OpCode::FastCall, (uint8_t) mainIdx, 0, 0), mainFnLoc);
        }

        emit(Op::iAsBx(OpCode::Exit, 0, 0), &program->nodes.back()->location);

        return module;
    }

    int Compiler::getGlobalID(const String &name)
    {
        if (globalIDMap.contains(name))
            return globalIDMap[name];
        int id            = (int) globalIDMap.size();
        globalIDMap[name] = id;
        return id;
    }

    Result<Register, Error> Compiler::allocateReg(const SourceLocation &loc)
    {
        if (current->freereg >= MAX_REGISTERS)
        {
            return std::unexpected(
                Error(ErrorType::RegisterOverflow, "too many registers", "", loc));
        }

        Register reg = current->freereg++;
        if (reg >= current->proto->maxRegisters)
        {
            current->proto->maxRegisters = reg + 1;
        }
        return reg;
    }

    void Compiler::freeReg(Register count)
    {
        if (current->freereg >= count)
        {
            current->freereg -= count;
        }
    }

    int Compiler::addConstant(Value val)
    {
        if (current->constantMap.contains(val))
            return current->constantMap[val];
        int idx = (int) current->proto->constants.size();
        current->proto->constants.push_back(val);
        current->constantMap[val] = idx;
        return idx;
    }

    void Compiler::emit(Instruction inst, SourceLocation *loc)
    {
        current->proto->code.push_back(inst);
        current->proto->locations.push_back(loc);
    }
} // namespace Fig
