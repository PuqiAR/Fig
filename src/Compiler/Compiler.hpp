/*!
    @file src/Compiler/Compiler.hpp
    @brief 编译器定义：物理直连 Bootstrapper
*/

#pragma once

#include <Ast/Ast.hpp>
#include <Bytecode/Bytecode.hpp>
#include <Error/Diagnostics.hpp>
#include <Object/Object.hpp>

namespace Fig
{
    using Register = std::uint8_t;

    class Compiler
    {
    private:
        static constexpr Register MAX_REGISTERS = 250;
        static constexpr Register NO_REG        = 255;

        struct FuncState
        {
            Proto    *proto;
            Register  freereg;
            FuncState *enclosing;
            HashMap<Value, int> constantMap;

            FuncState(Proto *p, FuncState *e)
                : proto(p), freereg(p->numParams), enclosing(e) {}
        };

        FuncState      *current = nullptr;
        CompiledModule *module  = nullptr;
        SourceManager  &manager;
        Diagnostics    &diag;

        HashMap<String, int> globalIDMap;
        int getGlobalID(const String& name);

        Result<Register, Error> allocateReg(const SourceLocation &loc);
        void                    freeReg(Register count = 1);
        int                     addConstant(Value val);

        void emit(Instruction inst, SourceLocation *loc);

        Result<void, Error>     compileStmt(Stmt *stmt);
        Result<Register, Error> compileExpr(Expr *expr, Register target = NO_REG);

    public:
        Compiler(SourceManager &m, Diagnostics &d) : manager(m), diag(d) {}
        Result<CompiledModule *, Error> Compile(Program *program);
    };
}
