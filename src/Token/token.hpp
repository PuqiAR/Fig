#pragma once

#include <cstdint>
#include <format>
#include <Utils/magic_enum/magic_enum.hpp>

#include <Core/fig_string.hpp>

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

        // TypeNull,   // Null
        // TypeInt,    // Int
        // TypeString, // String
        // TypeBool,   // Bool
        // TypeDouble, // Double

        /* Literal Types (not keyword)*/
        LiteralNumber, // number (int,float...)
        LiteralString, // FString
        LiteralBool,   // bool (true/false)
        LiteralNull,   // null (Null unique instance)

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
        friend bool operator==(const Token &l, const Token &r);

    private:
        FString value;
        TokenType type;

    public:
        size_t line, column;

        inline Token() {};
        inline Token(const FString &_value, TokenType _type) :
            value(_value), type(_type) {}
        inline Token(const FString &_value, TokenType _type, size_t _line, size_t _column) :
            value(_value), type(_type)
        {
            line = _line;
            column = _column;
        }
        const Token& setPos(size_t _line, size_t _column)
        {
            line = _line;
            column = _column;
            return *this;
        }
        size_t getLength()
        {
            return value.length();
        }
        const FString& getValue() const
        {
            return value;
        }
        inline FString toString() const
        {
            return FString(std::format(
                "Token('{}',{})",
                this->value.toBasicString(),
                magic_enum::enum_name(type)));
        }

        bool isIdentifier()
        {
            return type == TokenType::Identifier;
        }
        bool isLiteral()
        {
            return type == TokenType::LiteralNull || type == TokenType::LiteralBool || type == TokenType::LiteralNumber || type == TokenType::LiteralString;
        }
        
        TokenType getType() const
        {
            return type;
        }
    };

    inline bool operator==(const Token &l, const Token &r)
    {
        return l.type == r.type and l.value == r.value;
    }

    static Token IllegalTok(u8"ILLEGAL", TokenType::Illegal);
    static Token EOFTok(u8"EOF", TokenType::EndOfFile);
} // namespace Fig