#include <Parser/Parser.hpp>
#include <iostream>

int main()
{
    using namespace Fig;

    String fileName = "[memory]";
    String filePath =
        "System" + fileName;

    SourceManager srcManager(filePath);

    String source = R"(
        var a = 10;
        var a: Int;
        var a := 200 * 30 + 2;
    )";

    Lexer lexer(source, fileName);

    Diagnostics diagnostics;
    Parser parser(lexer, srcManager, fileName, diagnostics);

    auto result = parser.Parse();
    if (!result)
    {
        ReportError(result.error(), srcManager);
        return 1;
    }

    diagnostics.EmitAll(srcManager);

    Program *program = *result;
    std::cout << "Parsed " << program->nodes.size() << " statements\n";
    for (size_t i = 0; i < program->nodes.size(); ++i)
    {
        std::cout << '[' << i << "] " << program->nodes[i]->toString() << '\n';
    }

    return 0;
}
