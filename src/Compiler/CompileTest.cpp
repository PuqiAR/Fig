#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Deps/Deps.hpp>
#include <Core/Core.hpp>
#include <SourceManager/SourceManager.hpp>
#include <Compiler/Compiler.hpp>

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
        std::cerr << "Couldn't read file";
        return 1;
    }

    Lexer lexer(manager.GetSource(), fileName);
    Parser parser(lexer, manager, fileName);

    const auto &program_result = parser.Parse();
    if (!program_result)
    {
        ReportError(program_result.error(), manager);
        return 1;
    }
    Program *program = *program_result;
    
    Compiler compiler(fileName, manager);
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
    return 0;
}