#include <Error/Error.hpp>
#include <Lexer/Lexer.hpp>
#include <Token/Token.hpp>


#include <iostream>

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

    while (true)
    {
        auto result = lexer.NextToken();
        if (!result.has_value())
        {
            ReportError(result.error(), manager);
            break;
        }
        const Token  &token  = *result;
        const String &lexeme = manager.GetSub(token.index, token.length);
        const auto   &type   = magic_enum::enum_name(token.type);
        if (token.type == TokenType::EndOfFile)
        {
            std::cout << "EOF: " << type << " at " << token.index << '\n';
            break;
        }
        std::cout << lexeme << " --> " << type << '\n';
    }
}