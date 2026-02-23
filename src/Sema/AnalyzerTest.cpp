#include <Sema/Analyzer.hpp>

#include <Deps/Deps.hpp>
#include <Error/Error.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <SourceManager/SourceManager.hpp>

#include <iostream>

int main()
{
    using namespace Fig;

    const String &fileName = "test.fig";
    const String &filePath = "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/test.fig";

    SourceManager manager(filePath);
    manager.Read();

    if (!manager.read)
    {
        std::cerr << "Read file failed \n";
        return 1;
    }

    Lexer lexer(manager.GetSource(), fileName);
    Parser parser(lexer, manager, fileName);

    const auto &result = parser.Parse();
    if (!result)
    {
        ReportError(result.error(), manager);
        return 1;
    }

    Program *program = *result;

    Analyzer analyzer(manager);

    const auto &analyzeResult = analyzer.Analyze(program);
    if (!analyzeResult)
    {
        ReportError(analyzeResult.error(), manager);
        return 1;
    }

    std::cout << "Analyze successfully, PROGRAM OK\n";
    return 0;
}