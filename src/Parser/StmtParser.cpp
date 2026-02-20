/*!
    @file src/Parser/StmtParser.hpp
    @brief 语法分析器(Pratt + 手动递归下降) 语句解析实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<BlockStmt *, Error> Parser::parseBlockStmt() // 当前token为 {
    {
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `{`
        BlockStmt     *stmt     = new BlockStmt();
        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "unclosed braces in block stmt",
                    "insert `}`",
                    location));
            }
            if (match(TokenType::RightBrace))
            {
                break;
            }
            const auto &result = parseStatement();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            stmt->nodes.push_back(*result);
        }
        return stmt;
    }
    Result<VarDecl *, Error> Parser::parseVarDecl(
        bool isPublic) // 由 parseStatement调用, 当前token为 var
    {
        state = State::ParsingVarDecl;

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `var`

        if (currentToken().type != TokenType::Identifier)
        {
            return std::unexpected(makeUnexpectTokenError("VarDecl", "var name", currentToken()));
        }
        const String &name = srcManager.GetSub(currentToken().index, currentToken().length);
        consumeToken(); // consume name

        Expr *typeSpeicifer = nullptr;
        if (match(TokenType::Colon)) // `:`
        {
            const auto &result = parseExpression(0, TokenType::Assign);
            if (!result)
            {
                return std::unexpected(result.error());
            }
            typeSpeicifer = *result;
        }

        Expr *initExpr = nullptr;
        if (match(TokenType::Assign))
        {
            const auto &result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            initExpr = *result;
        }
        if (!match(TokenType::Semicolon))
        {
            makeExpectSemicolonError();
        }
        VarDecl *varDecl = new VarDecl(isPublic, name, typeSpeicifer, initExpr, location);
        return varDecl;
    }

    Result<IfStmt *, Error> Parser::parseIfStmt() // 由 parseStatement调用, 当前token is if
    {
        state                   = State::ParsingIf;
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `if`

        Expr *cond = nullptr;
        if (match(TokenType::LeftParen)) // match and consume `(`
        {
            const Token &lpToken = prevToken();
            const auto  &result  = parseExpression(0, TokenType::RightParen, TokenType::LeftBrace);
            if (!result)
            {
                return std::unexpected(result.error());
            }
            if (!match(TokenType::RightParen))
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "unclosed parenthese in if condition",
                    "insert `)`",
                    makeSourceLocation(lpToken)));
            }
            cond = *result;
        }
        else
        {
            const auto &result = parseExpression(0, TokenType::LeftBrace);
            if (!result)
            {
                return std::unexpected(result.error());
            }
            cond = *result;
        }
        state = State::ParsingIf;

        if (currentToken().type != TokenType::LeftBrace)
        {
            return std::unexpected(
                makeUnexpectTokenError("IfStmt", "LeftBrace `{`", currentToken()));
        }
        const auto &result = parseBlockStmt();
        if (!result)
        {
            return std::unexpected(result.error());
        }
        BlockStmt *consequent = *result;

        DynArray<ElseIfStmt *> elifs;
        BlockStmt             *alternate = nullptr;

        while (match(TokenType::Else))
        {
            SourceLocation elseLocation = makeSourceLocation(prevToken());
            if (match(TokenType::If))
            {
                // else if
                if (alternate)
                {
                    return std::unexpected(Error(ErrorType::SyntaxError,
                        "else if after else",
                        "remove else if",
                        elseLocation));
                }

                Expr *cond = nullptr;

                if (match(TokenType::LeftParen)) // `(`
                {
                    const Token &lpToken = prevToken();
                    const auto  &result =
                        parseExpression(0, TokenType::RightParen, TokenType::LeftBrace);
                    if (!result)
                    {
                        return std::unexpected(result.error());
                    }
                    state = State::ParsingIf;
                    if (!match(TokenType::RightParen))
                    {
                        return std::unexpected(Error(ErrorType::SyntaxError,
                            "unclosed parenthese in if condition",
                            "insert `)`",
                            makeSourceLocation(lpToken)));
                    }
                    cond = *result;
                }
                else
                {
                    const auto &result = parseExpression(0, TokenType::LeftBrace);
                    if (!result)
                    {
                        return std::unexpected(result.error());
                    }
                    state = State::ParsingIf;
                    cond  = *result;
                }
                if (currentToken().type != TokenType::LeftBrace)
                {
                    return std::unexpected(
                        makeUnexpectTokenError("ElseIfStmt", "LeftBrace `{`", currentToken()));
                }
                const auto &result = parseBlockStmt();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                state                  = State::ParsingIf;
                BlockStmt  *consequent = *result;
                ElseIfStmt *elif       = new ElseIfStmt(cond, consequent, elseLocation);
                elifs.push_back(elif);
            }
            else
            {
                // else
                if (alternate)
                {
                    return std::unexpected(Error(ErrorType::SyntaxError,
                        "duplicate else in if stmt",
                        "remove it",
                        elseLocation));
                }
                if (currentToken().type != TokenType::LeftBrace)
                {
                    return std::unexpected(
                        makeUnexpectTokenError("ElseStmt", "LeftBrace `{`", currentToken()));
                }
                const auto &result = parseBlockStmt();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                state     = State::ParsingIf;
                alternate = *result;
            }
        }
        IfStmt *ifStmt = new IfStmt(cond, consequent, elifs, alternate, location);
        return ifStmt;
    }

    Result<Stmt *, Error> Parser::parseStatement()
    {
        state = State::Standby;
        if (currentToken().type == TokenType::Public)
        {
            consumeToken(); // consume `public`
            if (currentToken().type == TokenType::Variable)
            {
                return parseVarDecl(true);
            }
        }
        if (currentToken().type == TokenType::Variable)
        {
            return parseVarDecl(false);
        }
        if (currentToken().type == TokenType::If)
        {
            return parseIfStmt();
        }

        const auto &expr_result = parseExpression(0);
        if (!expr_result)
        {
            return std::unexpected(expr_result.error());
        }
        ExprStmt *exprStmt = new ExprStmt(*expr_result);
        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }
        return exprStmt;
    }
}; // namespace Fig