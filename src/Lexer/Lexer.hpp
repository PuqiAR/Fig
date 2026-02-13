/*!
    @file src/Lexer/Lexer.hpp
    @brief 词法分析器(materialized lexeme)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-13
*/

#pragma once

#include <Deps/Deps.hpp>
#include <Token/Token.hpp>
#include <Core/SourceLocations.hpp>

namespace Fig
{
    class SourceReader
    {
    private:
        String source;
        size_t index;

        SourcePosition pos;

    public:
        SourceReader()
        {
            index = 0;
            pos.line = pos.column = 0;
        }
        SourceReader(const String &_source) // copy
        {
            source = _source;
            index = 0;
            pos.line = pos.column = 0;
        }

        SourcePosition &currentPosition() { return pos; }

        inline char32_t current() const
        {
            assert(index < source.length() && "SourceReader: get current failed, index out of range");
            return source[index];
        }

        inline bool hasNext() const { return index < source.length(); }

        inline char32_t peek() const
        {
            assert((index + 1) < source.length() && "SourceReader: get peek failed, index out of range");
            return source[index + 1];
        }

        inline char32_t peekIf() const
        {
            if ((index + 1) < source.length()) { return source[index + 1]; }
            return 0xFFFD;
        }

        inline char32_t produce()
        {
            // returns current rune, then next
            char32_t c = current();
            next();
            return c;
        }

        inline void next()
        {
            assert(hasNext() && "SrcReader: next failed, need more runes");
            ++index;

            if (current() == U'\n')
            {
                ++pos.line;
                pos.column = 1;
            }
            else
            {
                ++pos.column;
            }
        }

        inline size_t currentIndex() const { return index; }

        inline bool isAtEnd() const { return index == source.length() - 1; }
    };

    class Lexer
    {
    public:
        enum State : uint8_t
        {
            Normal,
            Error
        };

    private:
        String fileName;
        SourceReader rd;

    protected:
        Token scanComments();
        Token scanIdentifierOrKeyword();

        Token scanNumberLiteral();
        Token scanStringLiteral();
        Token scanBoolLiteral();
        Token scanLiteralNull();

        Token scanPunct();
    public:
        State state = Normal;

        Lexer() {}
        Lexer(const String &source, String _fileName)
        {
            rd = SourceReader(source);
            fileName = std::move(_fileName);
        }

        Token NextToken();
    };
}; // namespace Fig