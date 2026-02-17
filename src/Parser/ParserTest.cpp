#include <Parser/Parser.hpp>
#include <iostream>

int main()
{
    using namespace Fig;

    String        fileName = "test.fig";
    String        filePath = "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/test.fig";
    SourceManager srcManager(filePath);

    String source = srcManager.Read();
    if (!srcManager.read)
    {
        std::cerr << "Couldn't read file";
        return 1;
    }

    Lexer lexer(source, fileName);
    Parser parser(lexer, srcManager, fileName);
    // const auto &result = parser.parseExpression();
    // if (!result)
    // {
    //     ReportError(result.error(), srcManager);
    //     return 1;
    // }
    // Expr *expr = *result;
    // std::cout << expr->toString() << '\n';
}