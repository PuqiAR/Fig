/*!
    @file src/Parser/Parser.hpp
    @brief 语法分析器(Pratt + 手动递归下降) 定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <Ast/Ast.hpp>
#include <Deps/Deps.hpp>
#include <Error/Error.hpp>
#include <Lexer/Lexer.hpp>
#include <Token/Token.hpp>
#include <Utils/Arena.hpp>

#include <cstddef>
#include <cstdlib>
#include <unordered_set>

namespace Fig
{
    class Parser
    {
    private:
        Arena          arena;
        Lexer         &lexer;
        SourceManager &srcManager;

        size_t          index = 0; // 当前 Token 在 buffer 中的下标
        DynArray<Token> buffer;    // 已从 Lexer 读取的 Token 缓存

        String fileName;
        bool   isEOF = false;

        // 惰性获取下一个 Token，跳过注释
        Token nextToken()
        {
            if (index + 1 < buffer.size())
                return buffer[++index];
            if (isEOF)
                return buffer[index];

            while (true)
            {
                auto result = lexer.NextToken();
                if (!result)
                {
                    ReportError(result.error(), srcManager);
                    std::exit(-1);
                }
                const Token &token = result.value();
                if (token.type == TokenType::Comments)
                    continue; // 惰性跳过注释

                if (token.type == TokenType::EndOfFile)
                    isEOF = true;
                buffer.push_back(token);
                index = buffer.size() - 1;
                return buffer[index];
            }
        }

        inline Token prevToken()
        {
            return (index > 0) ? buffer[index - 1] : buffer[0];
        }
        inline Token currentToken()
        {
            if (buffer.empty())
                return nextToken();
            return buffer[index];
        }

        // 惰性窥视后续 Token
        Token peekToken(size_t lookahead = 1)
        {
            size_t targetIndex = index + lookahead;
            while (targetIndex >= buffer.size() && !isEOF)
            {
                auto result = lexer.NextToken();
                if (!result)
                {
                    ReportError(result.error(), srcManager);
                    std::abort();
                }
                if (result->type == TokenType::Comments)
                    continue;
                if (result->type == TokenType::EndOfFile)
                    isEOF = true;
                buffer.push_back(*result);
            }
            return (targetIndex >= buffer.size()) ? buffer.back() : buffer[targetIndex];
        }

        inline Token consumeToken()
        {
            Token current = currentToken();
            if (current.type != TokenType::EndOfFile)
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
                ParsingReturn,
                ParsingBreak,
                ParsingContinue,
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

        std::unordered_set<TokenType> &getTerminators()
        {
            static std::unordered_set<TokenType> terminators(getBaseTerminators());
            return terminators;
        }
        void resetTermintors()
        {
            getTerminators() = getBaseTerminators();
        }

        bool shouldTerminate()
        {
            const Token &token = currentToken();
            if (getTerminators().contains(token.type))
                return true;
            for (auto it = stateStack.rbegin(); it < stateStack.rend(); ++it)
            {
                if (it->stopAt.contains(token.type))
                    return true;
            }
            return false;
        }

        DynArray<State> stateStack;
        State          &currentState()
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
                stateStack.pop_back();
        }

        struct StateProtector
        {
            Parser *p;
            StateProtector(Parser *_p, State _s) : p(_p)
            {
                p->pushState(_s);
            }
            ~StateProtector()
            {
                p->popState();
            }
        };

        SourceLocation makeSourceLocation(const Token &tok)
        {
            auto [line, column] = srcManager.GetLineColumn(tok.index);
            // 物理防爆盾：防止因解析错位导致的异常列号引起终端 OOM
            if (column > 5000)
                column = 1;
            return SourceLocation(SourcePosition(line, column, tok.length),
                fileName,
                "[internal parser]",
                magic_enum::enum_name(currentState().type).data());
        }

        inline Error makeUnexpectTokenError(const String &stmt, const String &exp, const Token &got)
        {
            return Error(ErrorType::SyntaxError,
                std::format(
                    "expect '{}' in {}, got `{}`", exp, stmt, magic_enum::enum_name(got.type)),
                "none",
                makeSourceLocation(got));
        }

        inline Error makeExpectSemicolonError()
        {
            return Error(ErrorType::SyntaxError,
                "expect ';' after statement",
                "insert ';'",
                makeSourceLocation(currentToken()));
        }

        Result<TypeExpr *, Error> parseTypeExpr();
        Result<TypeExpr *, Error> parseNamedTypeExpr();

        Result<Expr *, Error> parseExpression(BindingPower = 0);
        Result<Expr *, Error> parseLiteralExpr();
        Result<Expr *, Error> parseIdentiExpr();
        Result<Expr *, Error> parsePrefixExpr();
        Result<Expr *, Error> parseInfixExpr(Expr *);
        Result<Expr *, Error> parseIndexExpr(Expr *);
        Result<Expr *, Error> parseCallExpr(Expr *);
        Result<Expr *, Error> parseNewExpr();

        Result<BlockStmt *, Error>       parseBlockStmt();
        Result<VarDecl *, Error>         parseVarDecl(bool);
        Result<IfStmt *, Error>          parseIfStmt();
        Result<WhileStmt *, Error>       parseWhileStmt();
        Result<DynArray<Param *>, Error> parseFnParams();
        Result<FnDefStmt *, Error>       parseFnDefStmt(bool);
        Result<ReturnStmt *, Error>      parseReturnStmt();

        Result<Stmt *, Error> parseStructDef(bool);
        Result<Stmt *, Error> parseInterfaceDef(bool);
        Result<Stmt *, Error> parseImpl();

        Result<Stmt *, Error> parseStatement();

    public:
        Parser(Lexer &_lexer, SourceManager &_src, String _file) :
            lexer(_lexer), srcManager(_src), fileName(std::move(_file))
        {
            pushState(State());
        }

        Result<Program *, Error> Parse();
    };

#define SET_STOP_AT(...) currentState().stopAt = {__VA_ARGS__};
} // namespace Fig
