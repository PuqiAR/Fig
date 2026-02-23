/*!
    @file src/Lexer/Lexer.cpp
    @brief 词法分析器(materialized lexeme)实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/
#include <Lexer/Lexer.hpp>

namespace Fig
{
    /*
        总则：
            Lexer不涉及语义部分，语义为Parser及之后的部分确定！
            确定边界 --> 分词
            无法确定 --> 错误的源，报错

    */

    Result<Token, Error> Lexer::scanComments()
    {
        Token tok(rd.currentIndex(), 2, TokenType::Comments);
        rd.skip(2); // 跳过 //
        do
        {
            tok.length++;
            if (rd.current() == U'\n')
            {
                rd.next(); // skip '\n'
                break;
            }
            rd.next();
        } while (rd.hasNext());
        return tok;
    }

    Result<Token, Error> Lexer::scanMultilineComments()
    {
        Token          tok(rd.currentIndex(), 2, TokenType::Comments);
        SourcePosition startPos = rd.currentPosition();
        rd.skip(2); // 跳过 / *
        while (true)
        {
            if (rd.isAtEnd())
            {
                return std::unexpected(Error(ErrorType::UnterminatedComments,
                    "unterminated multiline comments",
                    "insert '*/'",
                    makeSourceLocation(startPos)));
            }
            if (rd.current() == U'*' && rd.peekIf() == U'/')
            {
                rd.skip(2);
                break;
            }
            tok.length++;
            rd.next();
        }
        return tok;
    }

    Result<Token, Error> Lexer::scanIdentifierOrKeyword()
    {
        Token  tok(rd.currentIndex(), 1, TokenType::Identifier);
        String value;                  // 用于判断是标识符还是关键字
        value.push_back(rd.produce()); // 加入第一个

        while (!rd.isAtEnd() && CharUtils::isIdentifierContinue(rd.current())) // continue: _ / 0-9 / aA - zZ
        {
            tok.length++;
            value.push_back(rd.produce());
            if (rd.isAtEnd())
            {
                break;
            }
        }

        if (Token::keywordMap.contains(value))
        {
            tok.type = Token::keywordMap.at(value);
        }
        return tok;
    }

    Result<Token, Error> Lexer::scanNumberLiteral()
    {
        Token tok(rd.currentIndex(), 0, TokenType::LiteralNumber);
        state = State::ScanDec;

        if (rd.current() == U'0')
        {
            char32_t _peek = std::tolower(rd.peekIf());
            if (_peek == U'b')
            {
                state = State::ScanBin;
                rd.skip(2); // 跳过 0b
                tok.length += 2;
            }
            else if (_peek == U'x')
            {
                state = State::ScanHex;
                rd.skip(2); // 跳过 0x
                tok.length += 2;
            }
            // else
            // {
            //     return std::unexpected(Error(ErrorType::InvalidNumberLiteral,
            //                                  std::format("bad number postfix 0{}", String(_peek)),
            //                                  "correct it",
            //                                  makeSourceLocation(rd.currentPosition())));

            // }
        }

        do
        {
            char32_t current = rd.current();
            if (state == State::ScanDec && !CharUtils::isDigit(current))
            {
                break;
            }
            if (state == State::ScanHex && !CharUtils::isHexDigit(current))
            {
                break;
            }
            if (state == State::ScanBin && current != U'0' && current != U'1')
            {
                // return std::unexpected(
                //     Error(ErrorType::InvalidNumberLiteral,
                //           std::format("invalid binary number literal, scanning '{}'", String(&current)),
                //           "correct it",
                //           makeSourceLocation(rd.currentPosition())));
                break;
            }
            tok.length++;
            rd.next();
        } while (!rd.isAtEnd());

        // 下划线表示法(1_000_000)
        while (!rd.isAtEnd() && state == State::ScanDec && (rd.current() == U'_' || CharUtils::isDigit(rd.current())))
        {
            tok.length++;
            rd.next();
        }

        // 小数点
        if (rd.currentIf() == U'.')
        {
            tok.length++;
            rd.next();

            if (!CharUtils::isDigit(rd.currentIf()))
            {
                return std::unexpected(Error(ErrorType::InvalidNumberLiteral,
                    "need matissa",
                    "insert matissa",
                    makeSourceLocation(rd.currentPosition())));
            }
            while (!rd.isAtEnd() && CharUtils::isDigit(rd.current()))
            {
                tok.length++;
                rd.next();
            }
        }

        // 科学计数法
        if (rd.currentIf() == U'e')
        {
            tok.length++;
            char32_t peek = rd.peekIf();
            if (peek == U'+' || peek == U'-') // ae+b, ae-b
            {
                tok.length++;
                rd.skip(2); // consume `e`, +/-
            }
            else if (CharUtils::isDigit(peek)) // aeb 情况
            {
                rd.next(); // `e`
            }
            if (!CharUtils::isDigit(rd.currentIf()))
            {
                return std::unexpected(Error(ErrorType::InvalidNumberLiteral,
                    "need exponent for scientific notation",
                    "insert exponent",
                    makeSourceLocation(rd.currentPosition())));
            }
            while (!rd.isAtEnd() && CharUtils::isDigit(rd.current()))
            {
                tok.length++;
                rd.next();
            }
        }
        return tok;
    }
    Result<Token, Error> Lexer::scanStringLiteral()
    {
        state = (rd.current() == U'"' ? State::ScanStringDQ : Lexer::State::ScanStringSQ);

        SourcePosition startPos = rd.currentPosition();

        Token tok(rd.currentIndex(), 1, TokenType::LiteralString); // "
        rd.next();                                                 // skip " / '

        while (true)
        {
            if (state == State::ScanStringDQ && rd.current() == U'"')
            {
                tok.length++;
                rd.next(); // skip '"'
                break;
            }
            else if (state == State::ScanStringSQ && rd.current() == U'\'')
            {
                tok.length++;
                rd.next(); // skip `'`
                break;
            }
            else if (rd.isAtEnd())
            {
                return std::unexpected(Error(ErrorType::UnterminatedString,
                    "unterminated string literal",
                    std::format("insert '{}'", String((state == State::ScanStringDQ ? "\"" : "'"))),
                    makeSourceLocation(startPos)));
            }
            else
            {
                tok.length++;
                rd.next();
            }
        }
        return tok;
    }
    Result<Token, Error> Lexer::scanPunct()
    {
        Token tok(rd.currentIndex(), 0, TokenType::Illegal);

        auto startsWith = [&](const String &prefix) -> bool {
            for (const auto &p : Token::punctMap)
            {
                const String &op = p.first;
                if (op.starts_with(prefix))
                    return true;
            }
            return false;
        };

        String sym;

        do
        {
            String candidate = sym + rd.current();
            if (startsWith(candidate))
            {
                rd.next();
                tok.length++;
                sym = candidate;
            }
            else
            {
                break;
            }
        } while (!rd.isAtEnd() && CharUtils::isPunct(rd.current()));

        if (!Token::punctMap.contains(sym))
        {
            return std::unexpected(Error(ErrorType::InvalidSymbol,
                std::format("invalid symbol `{}`", sym),
                "correct it",
                makeSourceLocation(rd.currentPosition())));
        }
        tok.type = Token::punctMap.at(sym);
        return tok;
    }

    void Lexer::skipWhitespaces()
    {
        while (!rd.isAtEnd())
        {
            char32_t current = rd.current();
            if (!CharUtils::isAsciiSpace(current)) // 检查 EOF
                break;
            rd.next();
        }
    }

    Result<Token, Error> Lexer::NextToken()
    {
        if (rd.isAtEnd())
        {
            return Token(rd.getEOFIndex(), 1, TokenType::EndOfFile);
        }
        if (rd.current() == U'/' && rd.peekIf() == U'/')
        {
            return scanComments();
        }
        else if (rd.current() == U'/' && rd.peekIf() == U'*')
        {
            return scanMultilineComments();
        }
        else if (CharUtils::isIdentifierStart(rd.current()))
        {
            return scanIdentifierOrKeyword();
        }
        else if (CharUtils::isDigit(rd.current()))
        {
            return scanNumberLiteral();
        }
        else if (rd.current() == U'"' || rd.current() == U'\'')
        {
            return scanStringLiteral();
        }
        else if (CharUtils::isPunct(rd.current()))
        {
            return scanPunct();
        }
        else if (CharUtils::isSpace(rd.current()))
        {
            skipWhitespaces();
            return NextToken();
        }
        else
        {
            return std::unexpected(Error(ErrorType::InvalidCharacter,
                std::format("invalid character '{}' (U+{})", String(rd.current()), static_cast<int>(rd.current())),
                "correct it",
                makeSourceLocation(rd.currentPosition())));
        }
    }
}; // namespace Fig