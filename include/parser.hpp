#pragma once

#include <ast.hpp>
#include <lexer.hpp>
#include <fig_string.hpp>
#include <error.hpp>

#include <print>
#include <unordered_map>

#include <stack>

namespace Fig
{

    class Parser
    {
    private:
        Lexer lexer;
        std::vector<Ast::AstBase> output;
        std::vector<Token> previousTokens;

        size_t tokenPruduced = 0;
        size_t currentTokenIndex = 0;

        std::unique_ptr<AddressableError> error;

        Ast::AstAddressInfo currentAAI;

        std::stack<Ast::Expression> exprStack;

        bool needSemicolon = true;

        class SemicolonDisabler
        {
            Parser *p;
            bool original;

        public:
            SemicolonDisabler(Parser *parser) :
                p(parser), original(p->needSemicolon)
            {
                p->needSemicolon = false;
            }
            ~SemicolonDisabler()
            {
                p->needSemicolon = original;
            }
            // disable copy and assign
            SemicolonDisabler(const SemicolonDisabler &) = delete;
            SemicolonDisabler &operator=(const SemicolonDisabler &) = delete;
        };

        void pushNode(const Ast::AstBase &_node)
        {
            Ast::AstBase node = _node;
            node->setAAI(currentAAI);
            output.push_back(std::move(node));
        }
        void pushNode(const Ast::AstBase &_node, Ast::AstAddressInfo _aai)
        {
            Ast::AstBase node = _node;
            node->setAAI(_aai);
            output.push_back(node);
        }

        bool isTokenSymbol(Token tok)
        {
            return Lexer::symbol_map.contains(tok.getValue());
        }
        bool isTokenOp(Token tok)
        {
            return Ast::TokenToOp.contains(tok.getType());
        }
        bool isEOF()
        {
            if (tokenPruduced == 0) return false;
            return currentToken() == EOFTok;
        }

    public:
        using Precedence = uint32_t;
        static const std::unordered_map<Ast::Operator, std::pair<Precedence, Precedence>> opPrecedence;
        Parser(const Lexer &_lexer) :
            lexer(_lexer)
        {
        }

        AddressableError* getError() const
        {
            return error.get();
        }

        template <class _ErrT, typename = AddressableError>
        void throwAddressableError(FStringView msg, size_t line, size_t column, std::source_location loc = std::source_location::current())
        {
            static_assert(std::is_base_of_v<AddressableError, _ErrT>,
                          "_ErrT must derive from AddressableError");
            _ErrT spError(msg, line, column, loc);
            error = std::make_unique<_ErrT>(spError);
            throw spError;
        }
        template <class _ErrT, typename = AddressableError>
        void throwAddressableError(FStringView msg, std::source_location loc = std::source_location::current())
        {
            static_assert(std::is_base_of_v<AddressableError, _ErrT>,
                          "_ErrT must derive from AddressableError");
            // line, column provide by `currentAAI`
            _ErrT spError(msg, currentAAI.line, currentAAI.column, loc);
            error = std::make_unique<_ErrT>(spError);
            throw spError;
        }

        template <class _ErrT, typename = UnaddressableError>
        void throwUnaddressableError(FStringView msg, std::source_location loc = std::source_location::current())
        {
            static_assert(std::is_base_of_v<AddressableError, _ErrT>,
                          "_ErrT must derive from AddressableError");
            _ErrT spError(msg, loc);
            error = std::make_unique<_ErrT>(spError);
            throw spError;
        }

        void setCurrentAAI(Ast::AstAddressInfo _aai)
        {
            currentAAI = std::move(_aai);
        }

        Ast::AstAddressInfo getCurrentAAI() const
        {
            return currentAAI;
        }

        inline const Token &nextToken()
        {
            // 没有Rollback时, 存在 currentTokenIndex = tokenPruduced - 1
            next();
            return currentToken();
        }
        inline void rollback()
        {
            if (int64_t(currentTokenIndex - 1) < int64_t(0))
            // 同下 next注释
            {
                throw std::runtime_error("Internal Error in Parser::rollbackToken, trying to rollback but it's already on the begin");
            }
            currentTokenIndex--;
        }
        inline void next()
        {
            if (int64_t(currentTokenIndex) < (int64_t(tokenPruduced) - 1))
            {
                /*
                必须两个都显示转换为int64_t.否则，负数时会超出范围，变成int64_t max, 并且 CTI也需要显示转换，否则转换完的pruduced又会被转回去，变为 int64_t max
                */
                currentTokenIndex++;
                setCurrentAAI(Ast::AstAddressInfo{.line = currentToken().line, .column = currentToken().column});
                return;
            }
            if (isEOF()) return;
            const Token &tok = lexer.nextToken();
            tokenPruduced++;
            if (tok == IllegalTok) throw lexer.getError();
            currentTokenIndex = tokenPruduced - 1;
            setCurrentAAI(Ast::AstAddressInfo{.line = tok.line, .column = tok.column});
            previousTokens.push_back(tok);
        }
        inline const Token &currentToken()
        {
            if (tokenPruduced == 0) return nextToken();
            return previousTokens.at(currentTokenIndex);
        }
        inline Token rollbackToken()
        {
            rollback();
            return previousTokens.at(currentTokenIndex);
        }

        inline Token peekToken()
        {
            Token tok = nextToken();
            rollback();
            return tok;
        }

        std::pair<Precedence, Precedence> getBindingPower(Ast::Operator op)
        {
            return opPrecedence.at(op);
        }
        Precedence getLeftBindingPower(Ast::Operator op)
        {
            return getBindingPower(op).first;
        }
        Precedence getRightBindingPower(Ast::Operator op)
        {
            return getBindingPower(op).second;
        }

        // template <class _Tp, class... Args>
        // std::shared_ptr<_Tp> makeAst(Args &&...args)
        // {
        //     _Tp node(std::forward<Args>(args)...);
        //     node.setAAI(currentAAI);
        //     return std::shared_ptr<_Tp>(new _Tp(node));
        // }
        template <class _Tp, class... Args>
        std::shared_ptr<_Tp> makeAst(Args &&...args)
        {
            std::shared_ptr<_Tp> ptr = std::make_shared<_Tp>(std::forward<Args>(args)...);
            ptr->setAAI(currentAAI);
            return ptr;
        }

        void expectPeek(TokenType type)
        {
            if (peekToken().getType() != type)
            {
                throwAddressableError<SyntaxError>(FStringView(std::format("Expected `{}`, but got `{}`",
                                                                           magic_enum::enum_name(type),
                                                                           magic_enum::enum_name(peekToken().getType()))));
            }
        }

        void expect(TokenType type)
        {
            if (currentToken().getType() != type)
            {
                throwAddressableError<SyntaxError>(FStringView(std::format("Expected `{}`, but got `{}`",
                                                                           magic_enum::enum_name(type),
                                                                           magic_enum::enum_name(currentToken().getType()))));
            }
        }

        void expectPeek(TokenType type, FString expected)
        {
            if (peekToken().getType() != type)
            {
                throwAddressableError<SyntaxError>(FStringView(std::format("Expected `{}`, but got `{}`",
                                                                           expected.toBasicString(),
                                                                           magic_enum::enum_name(peekToken().getType()))));
            }
        }

        void expect(TokenType type, FString expected)
        {
            if (currentToken().getType() != type)
            {
                throwAddressableError<SyntaxError>(FStringView(std::format("Expected `{}`, but got `{}`",
                                                                           expected.toBasicString(),
                                                                           magic_enum::enum_name(currentToken().getType()))));
            }
        }

        [[nodiscard]] SemicolonDisabler disableSemicolon()
        {
            return SemicolonDisabler(this);
        }

        void expectSemicolon()
        {
            // if need semicolon and stream has `;`, consume it. if not need semicolon, do nothing

            if (!needSemicolon)
            {
                // disabled semicolon check
                if (currentToken().getType() == TokenType::Semicolon)
                {
                    next(); // consume `;`
                }
                return;
            }

            // normal semicolon check
            expectConsume(TokenType::Semicolon);
        }

        void expectConsume(TokenType type, FString expected)
        {
            expect(type, expected);
            next();
        }

        void expectConsume(TokenType type)
        {
            expect(type);
            next();
        }

        bool isNext(TokenType type)
        {
            return peekToken().getType() == type;
        }
        bool isThis(TokenType type)
        {
            return currentToken().getType() == type;
        }

        static constexpr FString varDefTypeFollowed = u8"(Followed)";

        Ast::VarDef __parseVarDef(bool); // entry: current is keyword `var` or `const` (isConst: Bool)
        Value __parseValue();
        Ast::ValueExpr __parseValueExpr();
        Ast::FunctionParameters __parseFunctionParameters(); // entry: current is Token::LeftParen
        Ast::BlockStatement __parseBlockStatement();         // entry: current is Token::LeftBrace
        Ast::If __parseIf();                                 // entry: current is Token::If
        Ast::While __parseWhile();                           // entry: current is Token::While
        Ast::Statement __parseIncrementStatement();          // only allowed in __parseFor function
        Ast::For __parseFor();                               // entry: current is Token::For
        Ast::Return __parseReturn();                         // entry: current is Token::Return
        Ast::Break __parseBreak();                           // entry: current is Token::Break
        Ast::Continue __parseContinue();                     // entry: current is Token::Continue

        Ast::VarExpr __parseVarExpr(FString);
        Ast::FunctionDef __parseFunctionDef(bool); // entry: current is Token::Identifier (isPublic: Bool)
        Ast::StructDef __parseStructDef(bool);     // entry: current is Token::Identifier (struct name) arg(isPublic: bool)

        Ast::BinaryExpr __parseInfix(Ast::Expression, Ast::Operator, Precedence);
        Ast::UnaryExpr __parsePrefix(Ast::Operator, Precedence);
        Ast::Expression __parseCall(Ast::Expression);
        
        Ast::ListExpr __parseListExpr(); // entry: current is `[`

        Ast::MapExpr __parseMapExpr(); // entry: current is `{`
        Ast::InitExpr __parseInitExpr(FString);  // entry: current is `{`, ahead is struct name. arg (struct name : FString)
        Ast::Expression __parseTupleOrParenExpr();             // entry: current is `(`

        Ast::FunctionLiteralExpr __parseFunctionLiteralExpr(); // entry: current is Token::LParen after Token::Function
        
        Ast::Statement __parseStatement();                     // entry: (idk)
        Ast::Expression parseExpression(Precedence, TokenType = TokenType::Semicolon, TokenType = TokenType::Semicolon);
        std::vector<Ast::AstBase> parseAll();
    };
}; // namespace Fig