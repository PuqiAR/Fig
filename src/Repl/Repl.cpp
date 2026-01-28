#include "Ast/astBase.hpp"
#include "Error/error.hpp"
#include "Error/errorLog.hpp"
#include <Core/fig_string.hpp>
#include <Repl/Repl.hpp>
#include <vector>

namespace Fig
{
    void Repl::Start() const
    {
        ostream << getPrompt() << "\n";

        const FString &sourcePath = u8"<stdin>";
        const std::vector<FString> &sourceLines{};

        Evaluator evaluator;

        evaluator.CreateGlobalContext();
        evaluator.RegisterBuiltinsValue();
        evaluator.SetSourcePath(sourcePath);
        evaluator.SetSourceLines(sourceLines);

        while (true)
        {
            ostream << "\r\n>>";
            const FString &line = readline();

            if (line.empty())
            {
                ostream << Object::getNullInstance()->toString().toBasicString();
                continue;
            }
            if (line == u8"!exit")
            {
                break;
            }

            Lexer lexer(line, sourcePath, sourceLines);
            Parser parser(lexer, sourcePath, sourceLines);

            std::vector<AstBase> program;
            try
            {
                program = parser.parseAll();

                StatementResult sr = evaluator.Run(program);
                ObjectPtr result = sr.result;
                ostream << result->toString().toBasicString() << '\n';
            }
            catch (AddressableError &e)
            {
                ostream << "Oops!\n"; 
                ErrorLog::logAddressableError(e);
                ostream << "\n";
            }
            catch (UnaddressableError &e)
            {
                ostream << "Oops!\n";
                ErrorLog::logUnaddressableError(e);
                ostream << "\n";
            }
        }
    }
} // namespace Fig