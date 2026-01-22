#include <Core/fig_string.hpp>
#include <Error/error.hpp>
#include <Token/token.hpp>
#include <Lexer/lexer.hpp>

#include <Core/fig_string.hpp>
#include <Utils/utils.hpp>

#if 0
    #include <iostream> // debug
#endif

#ifndef SourceInfo
    #define SourceInfo(ptr) (ptr->sourcePath), (ptr->sourceLines)
#endif

namespace Fig
{

    const std::unordered_map<FString, TokenType> Lexer::symbol_map{
        // 三字符
        {FString(u8"..."), TokenType::TripleDot},
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
        {FString(u8"}"), TokenType::RightBrace},
        {FString(u8"?"), TokenType::Question},
        {FString(u8"!"), TokenType::Not},
    };

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
        {FString(u8"new"), TokenType::New},
        {FString(u8"struct"), TokenType::Struct},
        {FString(u8"interface"), TokenType::Interface},
        {FString(u8"impl"), TokenType::Implement},
        {FString(u8"is"), TokenType::Is},
        {FString(u8"public"), TokenType::Public},
        {FString(u8"return"), TokenType::Return},
        {FString(u8"break"), TokenType::Break},
        {FString(u8"continue"), TokenType::Continue},
        {FString(u8"try"), TokenType::Try},
        {FString(u8"catch"), TokenType::Catch},
        {FString(u8"throw"), TokenType::Throw},
        {FString(u8"Finally"), TokenType::Finally},

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
                    error = SyntaxError(u8"Unterminated FString", this->line, it.column(), SourceInfo(this));
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
                    error = SyntaxError(FString(std::format("Unsupported escape character: {}",
                                                            FString(ec.getString()).toBasicString())),
                                        this->line,
                                        it.column(),
                                        SourceInfo(this));
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
            error = SyntaxError(u8"Unterminated FString", this->line, str_start_col, SourceInfo(this));
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
            error = SyntaxError(u8"Unterminated FString", this->line, str_start_col, SourceInfo(this));
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
                    error = SyntaxError(u8"Unterminated FString", this->line, it.column(), SourceInfo(this));
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
                    error = SyntaxError(FString(std::format("Unsupported escape character: {}",
                                                            FString(ec.getString()).toBasicString())),
                                        this->line,
                                        it.column(),
                                        SourceInfo(this));
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
            error = SyntaxError(u8"Unterminated FString", this->line, str_start_col, SourceInfo(this));
            return IllegalTok;
        }
        return Token(str, TokenType::LiteralString);
    }
    Token Lexer::scanNumber()
    {
        FString numStr;
        bool hasPoint = false;

        while (hasNext())
        {
            UTF8Char ch = *it;

            if (ch.isDigit() || ch == U'e')
            {
                numStr += ch.getString();
                next();
            }
            else if (ch == U'-' && !numStr.empty() && (numStr.ends_with(U'e') || numStr.ends_with(U'E')))
            {
                numStr += ch.getString();
                next();
            }
            else if (ch == U'+' && !numStr.empty() && (numStr.ends_with(U'e') || numStr.ends_with(U'E')))
            {
                numStr += ch.getString();
                next();
            }
            else if (ch == U'.' && !hasPoint)
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
        if (numStr.empty()) { return IllegalTok; }

        if (numStr.ends_with(U'e'))
        {
            error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                this->line,
                                it.column(),
                                SourceInfo(this));
            return IllegalTok;
        }

        bool hasDigit = false;
        for (auto it = numStr.begin(); it != numStr.end(); ++it)
        {
            if (isdigit(*it))
            {
                hasDigit = true;
                break;
            }
        }

        if (!hasDigit)
        {
            error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                this->line,
                                it.column(),
                                SourceInfo(this));
            return IllegalTok;
        }

        size_t ePos = numStr.find(U'e');
        if (ePos != FString::npos)
        {
            if (ePos == 0)
            {
                error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                    this->line,
                                    it.column(),
                                    SourceInfo(this));
                return IllegalTok;
            }
            if (ePos + 1 >= numStr.length())
            {
                error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                    this->line,
                                    it.column(),
                                    SourceInfo(this));
                return IllegalTok;
            }
            bool hasDigitAfterE = false;
            for (size_t i = ePos + 1; i < numStr.length(); ++i)
            {
                UTF8Char c = std::u8string(1,numStr[i]);
                if (c == U'+' || c == U'-')
                {
                    if (i != ePos + 1)
                    {
                        error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                            this->line,
                                            it.column(),
                                            SourceInfo(this));
                        return IllegalTok;
                    }
                    continue;
                }

                if (c.isDigit()) { hasDigitAfterE = true; }
                else
                {
                    error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                        this->line,
                                        it.column(),
                                        SourceInfo(this));
                    return IllegalTok;
                }
            }

            if (!hasDigitAfterE)
            {
                error = SyntaxError(FString(std::format("Illegal number literal: {}", numStr.toBasicString())),
                                    this->line,
                                    it.column(),
                                    SourceInfo(this));
                return IllegalTok;
            }
        }

        return Token(numStr, TokenType::LiteralNumber);
    }
    Token Lexer::scanSymbol()
    {
        FString sym;
        UTF8Char ch = *it;
        sym += ch.getString();

        auto startsWith = [&](const FString &prefix) -> bool {
            for (const auto &p : symbol_map)
            {
                const FString &op = p.first;
                if (op.starts_with(prefix))
                    return true;
            }
            return false;
        };

        if (!startsWith(sym))
        {
            error = SyntaxError(FString(std::format("No such operator: {}", sym.toBasicString())),
                                this->line,
                                it.column(),
                                SourceInfo(this));
            next();
            return IllegalTok;
        }

        while (hasNext())
        {
            UTF8Char peek = it.peek();
            if (!peek.isPunct())
                break;

            FString candidate = sym + FString(peek.getString());

            if (startsWith(candidate))
            {
                next();
                sym = candidate;
            }
            else
            {
                break;
            }
        }

        if (!symbol_map.contains(sym))
        {
            error = SyntaxError(FString(std::format("No such operator: {}", sym.toBasicString())),
                                this->line,
                                it.column(),
                                SourceInfo(this));
            next();
            return IllegalTok;
        }
        // std::cerr << Token(sym, symbol_map.at(sym)).toString().toBasicString() << '\n;
        next();
        return Token(sym, symbol_map.at(sym));
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
                error =
                    SyntaxError(FString(u8"Unterminated multiline comment"), this->line, it.column(), SourceInfo(this));
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
                return Token(u8"/", this->symbol_map.at(u8"/")).setPos(last_line, last_column);
            }
            c = it.peek();
            if (c != U'/' and c != U'*')
            {
                next();
                return Token(u8"/", this->symbol_map.at(u8"/")).setPos(last_line, last_column);
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
            error =
                SyntaxError(FString(std::format("Cannot tokenize char: '{}'", FString(ch.getString()).toBasicString())),
                            this->line,
                            it.column(),
                            SourceInfo(this));
            if (hasNext())
            {
                next();
            }
            return IllegalTok.setPos(last_line, last_column);
        }
    }

} // namespace Fig