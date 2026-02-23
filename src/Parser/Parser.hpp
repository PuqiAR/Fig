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
            const auto &result = lexer.NextToken();
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
                const auto &result = lexer.NextToken();
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

        inline Error makeUnexpectTokenError(const String &stmtType, const String &expect, const Token &tokenGot, std::source_location loc = std::source_location::current())
        {
            return Error(
                ErrorType::SyntaxError,
                std::format("expect '{}' in {}, got `{}`", expect, stmtType, magic_enum::enum_name(tokenGot.type)),
                "none",
                makeSourceLocation(tokenGot),
                loc
            );
        }

        inline Error makeExpectSemicolonError(std::source_location loc = std::source_location::current())
        {
            return Error(
                ErrorType::SyntaxError,
                "expect ';' after statement",
                "insert ';'",
                makeSourceLocation(currentToken()),
                loc
            );
        }

    public:
        enum class State : std::uint8_t
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

        } state;

        Parser(Lexer &_lexer, SourceManager &_srcManager, String _fileName) :
            lexer(_lexer), srcManager(_srcManager), fileName(std::move(_fileName))
        {
            state = State::Standby;
        }

    private:
        SourceLocation makeSourceLocation(const Token &tok)
        {
            auto [line, column] = srcManager.GetLineColumn(tok.index);
            return SourceLocation(SourcePosition(line, column, tok.length),
                fileName,
                "[internal parser]",
                magic_enum::enum_name(state).data());
        }

        /* Expressions */
        Result<LiteralExpr *, Error> parseLiteralExpr(); // 当前token为literal时调用
        Result<IdentiExpr *, Error>  parseIdentiExpr();  // 当前token为Identifier调用

        Result<InfixExpr *, Error>  parseInfixExpr(Expr *); // 由 parseExpression递归调用, 当前token为op
        Result<PrefixExpr *, Error> parsePrefixExpr();      // 由 parseExpression递归调用, 当前token为op

        Result<IndexExpr *, Error> parseIndexExpr(Expr *); // 由 parseExpression调用, 当前token为 `[`
        Result<CallExpr *, Error>  parseCallExpr(Expr *);  // 由 parseExpression调用, 当前token为 `(`

        const std::unordered_set<TokenType> &getBaseTerminators();
        std::unordered_set<TokenType> &getTerminators();  // 返回固定的终止符
        void resetTermintors();
        bool                          shouldTerminate(); // 判断是否终结

        Result<Expr *, Error> parseExpression(BindingPower = 0, TokenType stop = TokenType::Semicolon, TokenType stop2 = TokenType::Semicolon);

        /* Statements */
        Result<BlockStmt *, Error> parseBlockStmt(); // 当前token为 {
        Result<VarDecl *, Error> parseVarDecl(bool); // 由 parseStatement调用, 当前token为 var
        Result<IfStmt *, Error> parseIfStmt(); // 由 parseStatement调用, 当前token is if
        Result<Stmt *, Error> parseStatement();

    public:
        Result<Program *, Error> Parse();
    };
}; // namespace Fig