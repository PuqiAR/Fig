/*!
    @file src/Parser/Parser.hpp
    @brief 语法分析器(Pratt + 手动递归下降) 定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Ast/Ast.hpp>
#include <Deps/Deps.hpp>
#include <Error/Error.hpp>
#include <Lexer/Lexer.hpp>
#include <Token/Token.hpp>

#include <cstddef>
#include <cstdlib>

#include <unordered_set>

namespace Fig
{

    class Parser
    {
    private:
        Lexer         &lexer;
        SourceManager &srcManager;

        size_t          index = 0; // token在buffer下标
        DynArray<Token> buffer;

        String fileName;

        bool isEOF = false;

        Token nextToken()
        {
            assert(!isEOF && "nextToken: eof but called nextToken");
            if (index + 1 < buffer.size())
            {
                return buffer[++index];
            }
            auto result = lexer.NextToken();
            if (!result)
            {
                ReportError(result.error(), srcManager);
                std::exit(-1);
            }
            const Token &token = result.value();
            if (token.type == TokenType::EndOfFile)
            {
                isEOF = true;
            }
            buffer.push_back(token);
            index++;
            return token;
        }

        inline Token prevToken()
        {
            if (buffer.size() < 2)
            {
                return currentToken();
            }
            return buffer[buffer.size() - 2];
        }

        inline Token currentToken()
        {
            if (buffer.empty())
            {
                return nextToken();
            }
            return buffer.back();
        }

        Token peekToken(size_t lookahead = 1)
        {
            assert(!isEOF && "peekToken: eof but called peekToken");

            size_t peekIndex = index + lookahead;
            while (peekIndex >= buffer.size() && !isEOF)
            {
                auto result = lexer.NextToken();
                if (!result)
                {
                    ReportError(result.error(), srcManager);
                    std::abort();
                }
                const Token &token = result.value();
                if (token.type == TokenType::EndOfFile)
                {
                    isEOF = true;
                }
                buffer.push_back(token);
            }
            if (peekIndex >= buffer.size()) // 没有那么多token
            {
                return buffer.back(); // back是EOF Token
            }
            return buffer[peekIndex];
        }

        inline Token consumeToken()
        {
            if (isEOF)
                return buffer.back();
            Token current = currentToken();
            nextToken();
            return current;
        }

        inline bool match(TokenType type)
        {
            if (currentToken().type == type)
            {
                consumeToken();
                return true;
            }
            return false;
        }

        inline Error makeUnexpectTokenError(const String &stmtType,
            const String                                 &expect,
            const Token                                  &tokenGot,
            std::source_location                          loc = std::source_location::current())
        {
            return Error(ErrorType::SyntaxError,
                std::format("expect '{}' in {}, got `{}`",
                    expect,
                    stmtType,
                    magic_enum::enum_name(tokenGot.type)),
                "none",
                makeSourceLocation(tokenGot),
                loc);
        }

        inline Error makeExpectSemicolonError(
            std::source_location loc = std::source_location::current())
        {
            return Error(ErrorType::SyntaxError,
                "expect ';' after statement",
                "insert ';'",
                makeSourceLocation(currentToken()),
                loc);
        }

    public:
        struct State
        {
            enum StateType : std::uint8_t
            {
                Standby,

                ParsingLiteralExpr,
                ParsingIdentiExpr,

                ParsingInfixExpr,
                ParsingPrefixExpr,

                ParsingIndexExpr,
                ParsingCallExpr,

                ParsingVarDecl,
                ParsingIf,
                ParsingWhile,
                ParsingFnDefStmt,

                ParsingNamedTypeExpr,

            } type                               = StateType::Standby;
            std::unordered_set<TokenType> stopAt = {};
        };

    private:
        const std::unordered_set<TokenType> &getBaseTerminators()
        {
            static const std::unordered_set<TokenType> baseTerminators = {TokenType::Semicolon,
                TokenType::RightParen,
                TokenType::RightBracket,
                TokenType::RightBrace,
                TokenType::Comma,
                TokenType::EndOfFile};
            return baseTerminators;
        }

        std::unordered_set<TokenType>       &getTerminators() // 返回固定的终止符
        {
            /*
                Syntax terminators:
                ;  )  ]  }  ,  EOF
            */

            static std::unordered_set<TokenType> terminators(getBaseTerminators());
            return terminators;
        }

        void resetTermintors()
        {
            getTerminators() = getBaseTerminators();
        }
        bool shouldTerminate() // 判断是否终结
        {
            const Token &token       = currentToken();
            const auto  &terminators = getTerminators();

            if (terminators.contains(token.type))
            {
                return true;
            }
            for (auto it = stateStack.rbegin(); it < stateStack.rend(); ++it)
            {
                if (it->stopAt.contains(token.type))
                {
                    return true;
                }
            }
            return false;
        }

        DynArray<State> stateStack;

        State &currentState()
        {
            return stateStack.back();
        }

        void pushState(State _state)
        {
            stateStack.push_back(std::move(_state));
        }

        void popState()
        {
            if (!stateStack.empty())
            {
                stateStack.pop_back();
            }
        }

        class StateProtector
        {
            Parser *parser;

        public:
            StateProtector(Parser *p, const State &newState) : parser(p)
            {
                parser->pushState(newState);
            }

            ~StateProtector()
            {
                parser->popState();
            }

            // 禁止拷贝
            StateProtector(const StateProtector &)            = delete;
            StateProtector &operator=(const StateProtector &) = delete;
        };

    public:
        Parser(Lexer &_lexer, SourceManager &_srcManager, String _fileName) :
            lexer(_lexer), srcManager(_srcManager), fileName(std::move(_fileName))
        {
            pushState(State());
        }

    private:
        SourceLocation makeSourceLocation(const Token &tok)
        {
            auto [line, column] = srcManager.GetLineColumn(tok.index);
            return SourceLocation(SourcePosition(line, column, tok.length),
                fileName,
                "[internal parser]",
                magic_enum::enum_name(currentState().type).data());
        }

        /* TypeExpressions */

        Result<NamedTypeExpr *, Error> parseNamedTypeExpr(); // 当前token为identifier

        Result<TypeExpr *, Error> parseTypeExpr();

        /* Expressions */
        Result<LiteralExpr *, Error> parseLiteralExpr(); // 当前token为literal时调用
        Result<IdentiExpr *, Error>  parseIdentiExpr();  // 当前token为Identifier调用

        Result<InfixExpr *, Error> parseInfixExpr(
            Expr *);                                   // 由 parseExpression递归调用, 当前token为op
        Result<PrefixExpr *, Error> parsePrefixExpr(); // 由 parseExpression递归调用, 当前token为op

        Result<IndexExpr *, Error> parseIndexExpr(
            Expr *);                                     // 由 parseExpression调用, 当前token为 `[`
        Result<CallExpr *, Error> parseCallExpr(Expr *); // 由 parseExpression调用, 当前token为 `(`

        Result<Expr *, Error> parseExpression(BindingPower = 0);

        /* Statements */
        Result<BlockStmt *, Error> parseBlockStmt();   // 当前token为 {
        Result<VarDecl *, Error>   parseVarDecl(bool); // 由 parseStatement调用, 当前token为 var
        Result<IfStmt *, Error>    parseIfStmt();      // 由 parseStatement调用, 当前token为 if
        Result<WhileStmt *, Error> parseWhileStmt();   // 由 parseStatement调用, 当前token为 while

        Result<DynArray<Param *>, Error> parseFnParams();  // 由 parseFnDefStmt或lambda调用
        Result<FnDefStmt *, Error> parseFnDefStmt(bool);   // 由 parseStatement调用, 当前token为 func

        Result<Stmt *, Error>      parseStatement();

    public:
        Result<Program *, Error> Parse();
    };

#define SET_STOP_AT(...) currentState().stopAt = {__VA_ARGS__};
}; // namespace Fig