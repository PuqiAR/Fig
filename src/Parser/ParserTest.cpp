#include <Parser/Parser.hpp>
#include <iostream>

int main()
{
    using namespace Fig;

    String fileName = "test.fig";
    String filePath =
        "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/" + fileName;

    SourceManager srcManager(filePath);

    String source = srcManager.Read();
    if (!srcManager.read)
    {
        std::cerr << "Couldn't read file: " << filePath << '\n';
        return 1;
    }

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
