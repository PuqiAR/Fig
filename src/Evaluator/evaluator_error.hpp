#pragma once

#include <Error/error.hpp>
#include <Ast/astBase.hpp>

namespace Fig
{
    class EvaluatorError final : public AddressableError
    {
    public:
        FString typeName;
        using AddressableError::AddressableError;
        EvaluatorError(FString _typeName,
                       FString msg,
                       Ast::AstBase ast,
                       std::source_location loc = std::source_location::current())
        {
            message = msg;


            src_loc = std::move(loc);

            typeName = std::move(_typeName);

            if (ast != nullptr)
            {
                line = ast->getAAI().line;
                column = ast->getAAI().column;
                sourcePath = *ast->getAAI().sourcePath;
                sourceLines = *ast->getAAI().sourceLines;
            }

        }
        EvaluatorError(FString _typeName,
                       std::string_view msg,
                       Ast::AstBase ast,
                       std::source_location loc = std::source_location::current())
        {
            message = FString::fromBasicString(std::string(msg.data()));
            line = ast->getAAI().line;
            column = ast->getAAI().column;

            src_loc = std::move(loc);

            typeName = std::move(_typeName);

            sourcePath = *ast->getAAI().sourcePath;
            sourceLines = *ast->getAAI().sourceLines;
        }

        virtual FString getErrorType() const override { return typeName; }
    };
}; // namespace Fig