#include <Bytecode/Disassembler.hpp>
#include <Compiler/Compiler.hpp>
#include <Core/Core.hpp>
#include <Deps/Deps.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Sema/Analyzer.hpp>
#include <SourceManager/SourceManager.hpp>
#include <VM/VM.hpp>

#include <chrono>
#include <iostream>
#include <print>

int main()
{
    using namespace Fig;

    String fileName = "test.fig";
    String filePath = "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/test.fig";

    SourceManager manager(filePath);
    manager.Read();

    if (!manager.read)
    {
        std::cerr << "CRITICAL: Could not read source file: " << filePath << "\n";
        return 1;
    }

    Lexer  lexer(manager.GetSource(), fileName);
    Parser parser(lexer, manager, fileName);

    auto pRes = parser.Parse();
    if (!pRes)
    {
        ReportError(pRes.error(), manager);
        return 1;
    }
    Program *program = *pRes;

    Analyzer analyzer(manager);
    auto     aRes = analyzer.Analyze(program);
    if (!aRes)
    {
        ReportError(aRes.error(), manager);
        return 1;
    }
    std::cout << "Analyzer: Program OK\n";

    Diagnostics diag;
    Compiler    compiler(manager, diag);
    auto        cRes = compiler.Compile(program);
    if (!cRes)
    {
        ReportError(cRes.error(), manager);
        return 1;
    }

    CompiledModule *compiledModule = *cRes;

    Disassembler::DisassembleModule(compiledModule);

    VM vm;
    using Clock = std::chrono::high_resolution_clock;
    Clock clock;

    std::cout << "\n--- VM Execution Start ---\n";
    auto start   = clock.now();
    auto result_ = vm.Execute(compiledModule);
    auto end     = clock.now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    if (!result_)
    {
        ReportError(result_.error(), manager);
        return 1;
    }

    std::cout << "Result: " << (*result_).ToString() << "\n";
    std::cout << "Execution Cost: " << duration.count() << "ms\n";

    vm.PrintRegisters(); 
    vm.PrintGlobals();
    return 0;
}
