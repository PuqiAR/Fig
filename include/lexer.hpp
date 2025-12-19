#pragma once

#include <corecrt.h>
#include <cuchar>
#include <cwctype>
#include <unordered_map>
#include <vector>

#include <token.hpp>
#include <error.hpp>
#include <fig_string.hpp>
#include <utf8_iterator.hpp>
#include <warning.hpp>

namespace Fig
{

    class Lexer final
    {
    private:
        size_t line;
        const FString source;
        SyntaxError error;
        UTF8Iterator it;

        std::vector<Warning> warnings;

        size_t last_line, last_column, column = 1;

        bool hasNext()
        {
            return !this->it.isEnd();
        }

        void skipLine();
        inline void next()
        {
            if (*it == U'\n')
            {
                ++this->line;
                this->column = 1;
            }
            else
            {
                ++this->column;
            }
            ++it;
        }

        void pushWarning(size_t id, FString msg)
        {
            warnings.push_back(Warning(id, std::move(msg), getCurrentLine(), getCurrentColumn()));
        }
        void pushWarning(size_t id, FString msg, size_t line, size_t column)
        {
            warnings.push_back(Warning(id, std::move(msg), line, column));
        }

    public:
        static const std::unordered_map<FString, TokenType> symbol_map;
        static const std::unordered_map<FString, TokenType> keyword_map;

        inline Lexer(const FString &_source) :
            source(_source), it(source)
        {
            line = 1;
        }
        inline size_t getCurrentLine()
        {
            return line;
        }
        inline size_t getCurrentColumn()
        {
            return column;
        }
        SyntaxError getError() const
        {
            return error;
        }
        std::vector<Warning> getWarnings() const
        {
            return warnings;
        }

        Token nextToken();

        Token scanNumber();
        Token scanString();
        Token scanRawString();
        Token scanMultilineString();
        Token scanIdentifier();
        Token scanSymbol();
        Token scanComments();
    };
} // namespace Fig