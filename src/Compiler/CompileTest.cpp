#include <Parser/Parser.hpp>
#include <Sema/Analyzer.hpp>
#include <Compiler/Compiler.hpp>
#include <Bytecode/Disassembler.hpp>
#include <iostream>
#include <filesystem>

int main()
{
    using namespace Fig;

    String filePath = "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/tests/Compiler/test_basic.fig";

    if (!std::filesystem::exists(filePath.toStdString()))
    {
        std::cerr << "CRITICAL: Test file not found at: " << filePath << "\n";
        return 1;
    }

    SourceManager sm{filePath};
    String        source = sm.Read();

    if (!sm.read || source.length() == 0)
    {
        std::cerr << "CRITICAL: SourceManager failed to read: " << filePath << "\n";
        return 1;
    }

    Lexer  lexer(source, filePath);
    Parser parser(lexer, sm, filePath);

    auto pRes = parser.Parse();
    if (!pRes)
    {
        ReportError(pRes.error(), sm);
        return 1;
    }

    Program *program = *pRes;
    std::cout << "Successfully parsed nodes: " << program->nodes.size() << "\n";

    Analyzer analyzer(sm);
    auto     aRes = analyzer.Analyze(program);
    if (!aRes)
    {
        ReportError(aRes.error(), sm);
        return 1;
    }

    Diagnostics diag;
    Compiler    compiler(sm, diag);
    auto        cRes = compiler.Compile(program);
    if (!cRes)
    {
        ReportError(cRes.error(), sm);
        return 1;
    }

    // 使用正式的 Disassembler
    Disassembler::DisassembleModule(*cRes);

    return 0;
}
