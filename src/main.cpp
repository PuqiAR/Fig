#include <Compiler/Compiler.hpp>
#include <Core/Core.hpp>
#include <Deps/Deps.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <SourceManager/SourceManager.hpp>
#include <Sema/Analyzer.hpp>
#include <VM/VM.hpp>

#include <iostream>
#include <print>
#include <chrono>

int main()
{
    using namespace Fig;

    String fileName = "test.fig";
    String filePath = "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/test.fig";

    SourceManager manager(filePath);
    manager.Read();

    if (!manager.read)
    {
        std::cerr << "Couldn't read file";
        return 1;
    }

    Lexer  lexer(manager.GetSource(), fileName);
    Parser parser(lexer, manager, fileName);

    const auto &program_result = parser.Parse();
    if (!program_result)
    {
        ReportError(program_result.error(), manager);
        return 1;
    }
    Program *program = *program_result;

    Analyzer analyzer(manager);
    const auto &analyzeResult = analyzer.Analyze(program);
    if (!analyzeResult)
    {
        ReportError(analyzeResult.error(), manager);
        return 1;
    }
    std::cout << "analyzer: Program OK, PASSED\n";


    Compiler    compiler(fileName, manager);
    const auto &proto_result = compiler.Compile(program);
    if (!proto_result)
    {
        ReportError(proto_result.error(), manager);
        return 1;
    }

    Proto *proto = *proto_result;

    std::cout << "=== Constant Pool ===" << '\n';
    for (size_t i = 0; i < proto->constants.size(); ++i)
    {
        std::print("[{}] {}\n", i, proto->constants[i].ToString());
    }

    DumpCode(proto->code);

    std::cout << "\nMax Stack Size: " << (int) proto->maxStack << std::endl;
    
    VM vm;

    const auto &result_ = vm.Execute(proto);
    if (!result_)
    {
        ReportError(result_.error(), manager);
        return 1;
    }
    Value result = *result_;
    std::cout << "result: " << result.ToString() << "\n";
    vm.PrintRegisters();
}