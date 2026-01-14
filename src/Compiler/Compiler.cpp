#include <Ast/astBase.hpp>
#include <Compiler/Compiler.hpp>
#include <memory>

namespace Fig
{
    void Compiler::compile(Ast::Statement stmt)
    {
        using enum Ast::AstType;
        using namespace Ast;
        Ast::AstType type = stmt->getType();

        switch (type)
        {
            case VarDefSt: {
                auto vd = std::static_pointer_cast<VarDefAst>(stmt);
                const FString name = vd->name;
            }
        }
    }
}; // namespace Fig