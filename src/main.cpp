#include <Error/Error.hpp>
#include <Token/Token.hpp>

int main()
{
    using namespace Fig;
    Error error{ErrorType::MayBeNull,
                "unterminated string literal",
                "terminated it",
                SourceLocation{2, 4, 5, "main.cpp", "main", "main"}};
    SourceManager manager = SourceManager("T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/src/main.cpp");
    manager.Read();

    ReportError(error, manager);
}