/*!
    @file src/Compiler/Compiler.cpp
    @brief 编译器实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-18
*/

#include <Compiler/Compiler.hpp>

namespace Fig
{
    Result<Proto *, Error> Compiler::Compile(Program *program)
    {
        current->proto   = new Proto();
        current->freeReg = 0;

        for (Stmt *stmt : program->nodes)
        {
            auto result = compileStmt(static_cast<Stmt *>(stmt));
            if (!result)
            {
                return std::unexpected(result.error());
            }
        }
        Emit(Op::iABC(OpCode::Exit, 0, 0, 0)); // 一定要退出,这是虚拟机退出信号,否则ub
        return current->proto;
    }
}; // namespace Fig