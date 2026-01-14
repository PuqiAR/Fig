#pragma once

#include "Ast/astBase.hpp"
#include <Ast/ast.hpp>
#include <Bytecode/Bytecode.hpp>
#include <VMValue/VMValue.hpp>

#include <vector>

namespace Fig
{
    class Compiler
    {
    private:
        std::vector<Ast::Statement> source;
        std::vector<OpCodeType> output; // std::vector<uint8_t>

        std::vector<Value> constants;   
    public:
        std::vector<OpCodeType> getOutput() const { return output; }
        std::vector<Value> getConstantPool() const { return constants; }

        Compiler() {}
        Compiler(std::vector<Ast::Statement> _source) : source(std::move(_source)) {}

        void compile_expr(Ast::Expression);

        void compile(Ast::Statement);

        void CompileAll();
    };
}; // namespace Fig