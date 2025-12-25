#include <Core/fig_string.hpp>
#include <Error/error.hpp>
#include <Token/token.hpp>
#include <Lexer/lexer.hpp>

#include <Core/fig_string.hpp>
#include <Utils/utils.hpp>

namespace Fig
{

    const std::unordered_map<FString, TokenType> Lexer::symbol_map{
        // 双字符
        {FString(u8"=="), TokenType::Equal},
        {FString(u8"!="), TokenType::NotEqual},
        {FString(u8"<="), TokenType::LessEqual},
        {FString(u8">="), TokenType::GreaterEqual},
        {FString(u8"<<"), TokenType::ShiftLeft},
        {FString(u8">>"), TokenType::ShiftRight},
        {FString(u8"+="), TokenType::PlusEqual},
        {FString(u8"-="), TokenType::MinusEqual},
        {FString(u8"*="), TokenType::AsteriskEqual},
        {FString(u8"/="), TokenType::SlashEqual},
        {FString(u8"%="), TokenType::PercentEqual},
        {FString(u8"^="), TokenType::CaretEqual},
        {FString(u8"++"), TokenType::DoublePlus},
        {FString(u8"--"), TokenType::DoubleMinus},
        {FString(u8"&&"), TokenType::DoubleAmpersand},
        {FString(u8"||"), TokenType::DoublePipe},
        {FString(u8":="), TokenType::Walrus},
        {FString(u8"**"), TokenType::Power},
        {FString(u8"->"), TokenType::RightArrow},
        {FString(u8"=>"), TokenType::DoubleArrow},

        // 单字符
        {FString(u8"+"), TokenType::Plus},
        {FString(u8"-"), TokenType::Minus},
        {FString(u8"*"), TokenType::Asterisk},
        {FString(u8"/"), TokenType::Slash},
        {FString(u8"%"), TokenType::Percent},
        {FString(u8"^"), TokenType::Caret},
        {FString(u8"&"), TokenType::Ampersand},
        {FString(u8"|"), TokenType::Pipe},
        {FString(u8"~"), TokenType::Tilde},
        {FString(u8"="), TokenType::Assign},
        {FString(u8"<"), TokenType::Less},
        {FString(u8">"), TokenType::Greater},
        {FString(u8"."), TokenType::Dot},
        {FString(u8","), TokenType::Comma},
        {FString(u8":"), TokenType::Colon},
        {FString(u8";"), TokenType::Semicolon},
        {FString(u8"'"), TokenType::SingleQuote},
        {FString(u8"\""), TokenType::DoubleQuote},
        {FString(u8"("), TokenType::LeftParen},
        {FString(u8")"), TokenType::RightParen},
        {FString(u8"["), TokenType::LeftBracket},
        {FString(u8"]"), TokenType::RightBracket},
        {FString(u8"{"), TokenType::LeftBrace},
        {FString(u8"}"), TokenType::RightBrace}};

    const std::unordered_map<FString, TokenType> Lexer::keyword_map{
        {FString(u8"and"), TokenType::And},
        {FString(u8"or"), TokenType::Or},
        {FString(u8"not"), TokenType::Not},
        {FString(u8"import"), TokenType::Import},
        {FString(u8"func"), TokenType::Function},
        {FString(u8"var"), TokenType::Variable},
        {FString(u8"const"), TokenType::Const},
        // {FString(u8"final"), TokenType::Final},
        {FString(u8"while"), TokenType::While},
        {FString(u8"for"), TokenType::For},
        {FString(u8"if"), TokenType::If},
        {FString(u8"else"), TokenType::Else},
        {FString(u8"struct"), TokenType::Struct},
        {FString(u8"interface"), TokenType::Interface},
        {FString(u8"implement"), TokenType::Implement},
        {FString(u8"public"), TokenType::Public},
        {FString(u8"return"), TokenType::Return},
        {FString(u8"break"), TokenType::Break},
        {FString(u8"continue"), TokenType::Continue},


        // {FString(u8"Null"), TokenType::TypeNull},
        // {FString(u8"Int"), TokenType::TypeInt},
        // {FString(u8"String"), TokenType::TypeString},
        // {FString(u8"Bool"), TokenType::TypeBool},
        // {FString(u8"Double"), TokenType::TypeDouble},
    };
    void Lexer::skipLine()
    {
        while (*it != U'\n' and hasNext())
        {
            next();
        }
        next(); // skip '\n'
        ++line;
    }
    Token Lexer::scanIdentifier()
    {
        FString identifier;

        while (hasNext())
        {
            UTF8Char c = *it;
            if (c.isAlnum() || c == U'_')
            {
                identifier += c.getString();
                next();
            }
            else
            {
                break;
            }
        }
        if (this->keyword_map.contains(identifier))
        {
            return Token(identifier, this->keyword_map.at(identifier));
        }
        else if (identifier == u8"true" || identifier == u8"false")
        {
            return Token(identifier, TokenType::LiteralBool);
        }
        else if (identifier == u8"null")
        {
            // null instance
            return Token(identifier, TokenType::LiteralNull);
        }
        if (keyword_map.contains(Utils::toLower(identifier)))
        {
            pushWarning(1, identifier); // Identifier is too similar to a keyword or a primitive type
        }
        if (identifier.length() <= 1)
        {
            pushWarning(2, identifier); // The identifier is too abstract
        }
        return Token(identifier, TokenType::Identifier);
    }
    Token Lexer::scanString()
    {
        FString str;
        bool unterminated = true;
        size_t str_start_col = it.column() - 1;
        while (hasNext())
        {
            UTF8Char c = *it;
            if (c == U'"' || c == U'\n')
            {
                next();
                unterminated = false;
                break;
            }
            else if (c == U'\\') // c is '\'
            {
                if (it.isEnd())
                {
                    error = SyntaxError(u8"Unterminated FString", this->line, it.column());
                    return IllegalTok;
                }
                next();
                UTF8Char ec = *it;
                if (ec == U'n')
                {
                    next();
                    str += u8"\n";
                }
                else if (ec == U't')
                {
                    next();
                    str += u8"\t";
                }
                else if (ec == U'v')
                {
                    next();
                    str += u8"\v";
                }
                else if (ec == U'b')
                {
                    next();
                    str += u8"\b";
                }
                else if (ec == U'"')
                {
                    next();
                    str += u8"\"";
                }
                else if (ec == U'\'')
                {
                    next();
                    str += u8"'";
                }
                else
                {
                    error = SyntaxError(FString(
                                            std::format(
                                                "Unsupported escape character: {}",
                                                FString(ec.getString()).toBasicString())),
                                        this->line,
                                        it.column());
                    return IllegalTok;
                }
            }
            else
            {
                str += c.getString();
                next();
            }
        }
        if (unterminated)
        {
            error = SyntaxError(u8"Unterminated FString", this->line, str_start_col);
            return IllegalTok;
        }
        return Token(str, TokenType::LiteralString);
    }
    Token Lexer::scanRawString()
    {
        FString str;
        bool unterminated = true;
        size_t str_start_col = it.column() - 1;
        while (hasNext())
        {
            UTF8Char c = *it;
            if (c == U'"' || c == U'\n')
            {
                next();
                unterminated = false;
                break;
            }
            else
            {
                str += c.getString();
                next();
            }
        }
        if (unterminated)
        {
            error = SyntaxError(u8"Unterminated FString", this->line, str_start_col);
            return IllegalTok;
        }
        return Token(str, TokenType::LiteralString);
    }
    Token Lexer::scanMultilineString()
    {
        FString str;
        bool unterminated = true;

        uint8_t end = 0;
        size_t str_start_col = it.column() - 1;
        while (hasNext())
        {
            UTF8Char c = *it;
            if (c == U'"')
            {
                if (end == 3)
                {
                    next();
                    unterminated = false;
                    break;
                }
                end++;
                next();
                continue;
            }
            else if (c == U'\\') // c is '\'
            {
                if (it.isEnd())
                {
                    error = SyntaxError(u8"Unterminated FString", this->line, it.column());
                    return IllegalTok;
                }
                next();
                UTF8Char ec = *it;
                if (ec == U'n')
                {
                    next();
                    str += u8"\n";
                }
                else if (ec == U't')
                {
                    next();
                    str += u8"\t";
                }
                else if (ec == U'v')
                {
                    next();
                    str += u8"\v";
                }
                else if (ec == U'b')
                {
                    next();
                    str += u8"\b";
                }
                else if (ec == U'"')
                {
                    next();
                    str += u8"\"";
                }
                else if (ec == U'\'')
                {
                    next();
                    str += u8"'";
                }
                else if (ec == U'\\')
                {
                    next();
                    str += u8"\\";
                }
                else
                {
                    error = SyntaxError(FString(
                                            std::format(
                                                "Unsupported escape character: {}",
                                                FString(ec.getString()).toBasicString())),
                                        this->line,
                                        it.column());
                    return IllegalTok;
                }
            }
            else
            {
                str += c.getString();
            }
            end = 0;
        }
        if (unterminated)
        {
            error = SyntaxError(u8"Unterminated FString", this->line, str_start_col);
            return IllegalTok;
        }
        return Token(str, TokenType::LiteralString);
    }
    Token Lexer::scanNumber()
    {
        FString numStr;
        bool hasPoint = false;
        // 负号(减号) 直接交由 scanSymbol处理，在parser中被分类->与数字结合/变为操作数
        while (hasNext())
        {
            UTF8Char ch = *it;
            if (ch.isDigit() or ch == U'e') // . / e / - for scientific counting
            {
                numStr += ch.getString();
                next();
            }
            else if (ch == U'-' and numStr.ends_with(U'-'))
            {
                numStr += ch.getString();
                next();
            }
            else if (ch == U'.' and not hasPoint)
            {
                hasPoint = true;
                numStr += ch.getString();
                next();
            }
            else
            {
                break;
            }
        }
        // Numbers in Fig-lang
        /*
            114514
            1145.14
            1.14e3  -> 1140
            1.14e-3 -> 0.00114
            .3      -> 0.3
        */
        // checking legality
        if ((*numStr.end()) == u'e') // e 后面必须跟整数表示科学计数
        {
            error = SyntaxError(FString(
                                    std::format("Ellegal number literal: {}", numStr.toBasicString())),
                                this->line, it.column());
            return IllegalTok;
        }
        return Token(numStr, TokenType::LiteralNumber);
    }
    Token Lexer::scanSymbol()
    {
        FString sym;
        UTF8Char ch = *it;

        sym += ch.getString();
        UTF8Char peek = UTF8Char(u8"");
        if (hasNext() and (peek = it.peek()).isPunct()) // 窥探下一个操作符
        {
            FString symd = FString(sym + peek.getString());
            if (this->symbol_map.contains(symd))
            {
                // Operator length is 2
                next();
                sym = symd;
            }
            // Operator length is 1
            else if (!this->symbol_map.contains(sym))
            {
                // check legality
                error = SyntaxError(FString(
                                        std::format("No such a operator: {}", sym.toBasicString())),
                                    this->line, it.column());
            }
        }
        next();
        return Token(sym, this->symbol_map.at(sym)); // const object 'symbol_map', operator[] call is invalid
    }
    Token Lexer::scanComments()
    {
        // entry: when iterator current char is '/' and peek is '/' or '*'
        // current char is '/'
        FString comment;

        if (it.peek() == U'/') // single-line comment
        {
            next(); // skip first '/'
            next(); // skip second '/'

            UTF8Char c = *it;
            while (c != U'\n' and hasNext())
            {
                comment += c.getString();
                next();
                c = *it;
            }

            if (hasNext() && c == U'\n')
            {
                next();
            }
        }
        else // multi-line comment
        {
            next(); // skip '/'
            next(); // skip '*'

            UTF8Char c = *it;
            bool terminated = false;

            while (hasNext())
            {
                if (c == U'*' and hasNext() and it.peek() == U'/')
                {
                    next(); // skip '*'
                    next(); // skip '/'
                    terminated = true;
                    break;
                }
                else
                {
                    comment += c.getString();
                    next();
                    c = *it;
                }
            }

            if (!terminated)
            {
                error = SyntaxError(FString(u8"Unterminated multiline comment"), this->line, it.column());
                next();
                return IllegalTok;
            }
        }

        return Token(comment, TokenType::Comments);
    }
    Token Lexer::nextToken()
    {
        if (!hasNext())
        {
            return EOFTok;
        }
        UTF8Char ch = *it;
        while (ch.isSpace())
        {
            next();
            ch = *it;
            if (!hasNext())
            {
                return EOFTok.setPos(getCurrentLine(), getCurrentColumn());
            }
        }
        last_line = getCurrentLine();
        last_column = getCurrentColumn();
        if (ch == U'/')
        {
            UTF8Char c{u8""};
            if (!hasNext())
            {
                next();
                // return Token(u8"/", this->symbol_map.at(u8"/")).setPos(last_line, last_column);
            }
            c = it.peek();
            if (c != U'/' and c != U'*')
            {
                next();
                // return Token(u8"/", this->symbol_map.at(u8"/")).setPos(last_line, last_column);
            }
            scanComments().setPos(last_line, last_column);
            return nextToken();
            // now we ignore comments to avoid some stupid bugs
        }
        if (ch == U'r' and hasNext() and it.peek() == U'"')
        {
            // r""
            // raw FString
            next();
            next();
            return scanRawString().setPos(last_line, last_column);
        }
        if (ch.isAlpha() || ch == U'_')
        {
            return scanIdentifier().setPos(last_line, last_column);
        }
        else if (ch == U'"')
        {
            next();
            return scanString().setPos(last_line, last_column);
        }
        else if (ch.isDigit())
        {
            return scanNumber().setPos(last_line, last_column);
        }
        else if (ch.isPunct())
        {
            return scanSymbol().setPos(last_line, last_column);
        }
        else
        {
            error = SyntaxError(FString(
                                    std::format("Cannot tokenize char: '{}'", FString(ch.getString()).toBasicString())),
                                this->line, it.column());
            if (hasNext())
            {
                next();
            }
            return IllegalTok.setPos(last_line, last_column);
        }
    }

} // namespace Fig