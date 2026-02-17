/*!
    @file src/Lexer/Lexer.hpp
    @brief 词法分析器(materialized lexeme)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-13
*/

#pragma once

#include <Core/SourceLocations.hpp>
#include <Deps/Deps.hpp>
#include <Error/Error.hpp>
#include <Token/Token.hpp>


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
            index    = 0;
            pos.line = pos.column = 1;
        }
        SourceReader(const String &_source) // copy
        {
            source   = _source;
            index    = 0;
            pos.line = pos.column = 1;
        }

        SourcePosition &currentPosition()
        {
            return pos;
        }

        inline char32_t current() const
        {
            assert(index < source.length() && "SourceReader: get current failed, index out of range");
            return source[index];
        }

        inline char32_t currentIf() const
        {
            if (index >= source.length())
            {
                return U'\0';
            }
            return source[index];
        }

        inline bool hasNext() const
        {
            return index < source.length() - 1;
        }

        inline char32_t peek() const
        {
            assert((index + 1) < source.length() && "SourceReader: get peek failed, index out of range");
            return source[index + 1];
        }

        inline char32_t peekIf() const
        {
            if ((index + 1) < source.length())
            {
                return source[index + 1];
            }
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
            char32_t consumed = currentIf();

            ++index;
            if (consumed == U'\n')
            {
                ++pos.line;
                pos.column = 1;
            }
            else
            {
                ++pos.column;
            }
        }

        inline void skip(size_t n)
        {
            for (size_t i = 0; i < n; ++i)
            {
                next();
            }
        }

        inline size_t currentIndex() const
        {
            return index;
        }

        inline bool isAtEnd() const
        {
            return index >= source.length();
        }

        inline size_t getEOFIndex() const
        {
            return source.length();
        }
    };

    class Lexer
    {
    public:
        enum class State : uint8_t
        {
            Error,
            Standby,
            End,

            ScanComments,          // 单行注释
            ScanMultilineComments, // 多行注释
            ScanIdentifier,        // 关键字也算

            ScanDec,      // 十进制数字, 如 1.2 31, 3.14e+3, 1_000_0000
            ScanBin,      // 二进制数字, 如 0b0001 / 0B0001
            ScanHex,      // 十六进制数字, 如 0xABCD / 0XabCd
            ScanStringDQ, // 双引号字符串, 如 "hello, world!"
            ScanStringSQ, // 单引号字符串, 如 'hello'
            ScanBool,     // 布尔字面量, true / false
            ScanNull,     // 空值字面量, null

            ScanPunct, // 符号
        };

    private:
        String       fileName;
        SourceReader rd;

    protected:
        Result<Token, Error> scanComments();
        Result<Token, Error> scanMultilineComments();

        Result<Token, Error> scanIdentifierOrKeyword();

        Result<Token, Error> scanNumberLiteral();
        Result<Token, Error> scanStringLiteral(); // 支持多行
        // Result<Token, Error> scanBoolLiteral(); 由 scanIdentifier...扫描
        // Result<Token, Error> scanLiteralNull(); 由 scanIdentifier...扫描

        Result<Token, Error> scanPunct();

        void skipWhitespaces();

    public:
        State state = State::Standby;

        Lexer() {}
        Lexer(const String &source, String _fileName)
        {
            rd       = SourceReader(source);
            fileName = std::move(_fileName);
        }

        SourceLocation makeSourceLocation(SourcePosition current_pos)
        {
            current_pos.tok_length = 1;
            return SourceLocation(
                current_pos, fileName, "[internal lexer]", String(magic_enum::enum_name(state).data()));
        }

        Result<Token, Error> NextToken();
    };
}; // namespace Fig