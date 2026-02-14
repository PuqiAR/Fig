/*!
    @file src/Token/Token.hpp
    @brief Token定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <cstdint>
#include <format>

#include <Utils/magic_enum/magic_enum.hpp>

#include <Deps/Deps.hpp>
#include <Core/SourceLocations.hpp>

namespace Fig
{
    enum class TokenType : int8_t
    {
        Illegal = -1,
        EndOfFile = 0,

        Comments,

        Identifier,

        /* Keywords */
        Package,  // package
        And,      // and
        Or,       // or
        Not,      // not
        Import,   // import
        Function, // func
        Variable, // var
        Const,    // const
        // Final,     // final
        While,     // while
        For,       // for
        If,        // if
        Else,      // else
        New,       // new
        Struct,    // struct
        Interface, // interface
        Implement, // impl
        Is,        // is
        Public,    // public
        Return,    // return
        Break,     // break
        Continue,  // continue
        Try,       // try
        Catch,     // catch
        Throw,     // throw
        Finally,   // finally
        As,        // as

        // TypeNull,   // Null
        // TypeInt,    // Int
        // TypeDeps::String, // Deps::String
        // TypeBool,   // Bool
        // TypeDouble, // Double

        /* Literal Types */
        LiteralNumber, // number (int,float...)
        LiteralString, // string

        LiteralTrue,  // true  <-- keyword
        LiteralFalse, // false <-- keyword
        LiteralNull,  // null (Null unique instance) <-- keyword

        /* Punct */
        Plus,       // +
        Minus,      // -
        Asterisk,   // *
        Slash,      // /
        Percent,    // %
        Caret,      // ^
        Ampersand,  // &
        Pipe,       // |
        Tilde,      // ~
        ShiftLeft,  // <<
        ShiftRight, // >>
        // Exclamation,      // !
        Question,    // ?
        Assign,      // =
        Less,        // <
        Greater,     // >
        Dot,         // .
        Comma,       // ,
        Colon,       // :
        Semicolon,   // ;
        SingleQuote, // '
        DoubleQuote, // "
        // Backtick,         // `
        // At,               // @
        // Hash,             // #
        // Dollar,           // $
        // Backslash,        // '\'
        // Underscore,       // _
        LeftParen,    // (
        RightParen,   // )
        LeftBracket,  // [
        RightBracket, // ]
        LeftBrace,    // {
        RightBrace,   // }
        // LeftArrow,        // <-
        RightArrow,      // ->
        DoubleArrow,     // =>
        Equal,           // ==
        NotEqual,        // !=
        LessEqual,       // <=
        GreaterEqual,    // >=

        PlusEqual,       // +=
        MinusEqual,      // -=
        AsteriskEqual,   // *=
        SlashEqual,      // /=
        PercentEqual,    // %=
        CaretEqual,      // ^=
        
        DoublePlus,      // ++
        DoubleMinus,     // --
        DoubleAmpersand, // &&
        DoublePipe,      // ||
        Walrus,          // :=
        Power,           // **

        TripleDot, // ... for variadic parameter
    };

    class Token final
    {
    public:
        static const HashMap<String, TokenType> punctMap;
        static const HashMap<String, TokenType> keywordMap;

        size_t index, length;
        //      源文件中的下标 Token长度
        TokenType type;

        Token() : index(0), length(0), type(TokenType::Illegal) {};
        Token(size_t _index, size_t _length, TokenType _type) : index(_index), length(_length), type(_type) {}
        Deps::String toString() const
        {
            return Deps::String(std::format("Token'{}' at {}, len {}", magic_enum::enum_name(type), index, length));
        }

        bool isIdentifier() const { return type == TokenType::Identifier; }
        bool isLiteral() const
        {
            return type == TokenType::LiteralNull || type == TokenType::LiteralTrue || type == TokenType::LiteralFalse
                   || type == TokenType::LiteralNumber || type == TokenType::LiteralString;
        }

        Token &operator=(const Token &other)
        {
            if (this == &other)
            {
                return *this;
            }
            index = other.index;
            length = other.length;
            type = other.type;
            return *this;
        }
    };
} // namespace Fig