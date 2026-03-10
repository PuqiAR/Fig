/*!
    @file src/Parser/StmtParser.cpp
    @brief 语法分析器(Pratt + 手动递归下降) 语句解析实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<BlockStmt *, Error> Parser::parseBlockStmt()
    {
        SourceLocation location = makeSourceLocation(consumeToken());
        BlockStmt     *stmt     = arena.Allocate<BlockStmt>();
        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "unclosed braces in block stmt",
                    "insert '}'",
                    location));
            }
            if (match(TokenType::RightBrace))
            {
                break;
            }
            auto result = parseStatement();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            stmt->nodes.push_back(*result);
        }
        return stmt;
    }

    Result<VarDecl *, Error> Parser::parseVarDecl(bool isPublic)
    {
        StateProtector p(this, {State::ParsingVarDecl});

        SourceLocation location = makeSourceLocation(consumeToken());

        if (currentToken().type != TokenType::Identifier)
        {
            return std::unexpected(makeUnexpectTokenError("VarDecl", "var name", currentToken()));
        }
        const String &name = srcManager.GetSub(currentToken().index, currentToken().length);
        consumeToken();

        TypeExpr *typeSpeicifer = nullptr;
        if (match(TokenType::Colon))
        {
            auto result = parseTypeExpr();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            typeSpeicifer = *result;
        }

        Expr *initExpr = nullptr;
        bool  isInfer  = false;
        if (match(TokenType::Assign))
        {
            auto result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            initExpr = *result;
        }
        else if (match(TokenType::Walrus))
        {
            if (typeSpeicifer)
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "used type infer but specifying the type",
                    "change `:=` to '='",
                    makeSourceLocation(prevToken())));
            }
            auto result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            initExpr = *result;
            isInfer  = true;
        }
        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }
        VarDecl *varDecl =
            arena.Allocate<VarDecl>(isPublic, name, typeSpeicifer, isInfer, initExpr, location);
        return varDecl;
    }

    Result<IfStmt *, Error> Parser::parseIfStmt()
    {
        StateProtector p(this, {State::ParsingIf});

        SourceLocation location = makeSourceLocation(consumeToken());

        Expr *cond = nullptr;
        if (match(TokenType::LeftParen))
        {
            const Token &lpToken = prevToken();
            SET_STOP_AT(TokenType::RightParen, TokenType::LeftBrace);
            const auto &result = parseExpression(0);
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
            SET_STOP_AT(TokenType::LeftBrace);
            auto result = parseExpression(0);
            if (!result)
            {
                return std::unexpected(result.error());
            }
            cond = *result;
        }

        if (currentToken().type != TokenType::LeftBrace)
        {
            return std::unexpected(
                makeUnexpectTokenError("IfStmt", "LeftBrace `{`", currentToken()));
        }
        auto result = parseBlockStmt();
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
                if (alternate)
                {
                    return std::unexpected(Error(ErrorType::SyntaxError,
                        "else if after else",
                        "remove else if",
                        elseLocation));
                }

                Expr *cond = nullptr;

                if (match(TokenType::LeftParen))
                {
                    const Token &lpToken = prevToken();

                    SET_STOP_AT(TokenType::RightParen, TokenType::LeftBrace);
                    const auto &result = parseExpression(0);
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
                    SET_STOP_AT(TokenType::LeftBrace);
                    auto result = parseExpression(0);
                    if (!result)
                    {
                        return std::unexpected(result.error());
                    }
                    cond = *result;
                }
                if (currentToken().type != TokenType::LeftBrace)
                {
                    return std::unexpected(
                        makeUnexpectTokenError("ElseIfStmt", "LeftBrace `{`", currentToken()));
                }
                auto result = parseBlockStmt();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                BlockStmt  *consequent = *result;
                ElseIfStmt *elif       = arena.Allocate<ElseIfStmt>(cond, consequent, elseLocation);
                elifs.push_back(elif);
            }
            else
            {
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
                auto result = parseBlockStmt();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                alternate = *result;
            }
        }
        IfStmt *ifStmt = arena.Allocate<IfStmt>(cond, consequent, elifs, alternate, location);
        return ifStmt;
    }

    Result<WhileStmt *, Error> Parser::parseWhileStmt()
    {
        StateProtector p(this, {State::ParsingWhile});

        SourceLocation location = makeSourceLocation(consumeToken());

        Expr *cond = nullptr;
        if (match(TokenType::LeftParen))
        {
            const Token &lpToken = prevToken();
            SET_STOP_AT(TokenType::RightParen, TokenType::LeftBrace);

            auto result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }

            if (!match(TokenType::RightParen))
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "unclosed parenthese in while condition",
                    "insert ')'",
                    makeSourceLocation(lpToken)));
            }
            cond = *result;
        }
        else
        {
            SET_STOP_AT(TokenType::LeftBrace);
            auto result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            cond = *result;
        }

        if (currentToken().type != TokenType::LeftBrace)
        {
            return std::unexpected(
                makeUnexpectTokenError("while stmt", "left brace '{'", currentToken()));
        }

        auto result = parseBlockStmt();
        if (!result)
        {
            return std::unexpected(result.error());
        }
        BlockStmt *body = *result;

        WhileStmt *whileStmt = arena.Allocate<WhileStmt>(cond, body, location);
        return whileStmt;
    }

    Result<DynArray<Param *>, Error> Parser::parseFnParams()
    {
        StateProtector p(this, {State::ParsingFnDefStmt});

        const Token      &lpToken = consumeToken();
        DynArray<Param *> params;

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "unclosed parenthese in function parameters",
                    "insert ')'",
                    makeSourceLocation(lpToken)));
            }
            if (match(TokenType::RightParen))
            {
                break;
            }

            const Token   &nToken   = consumeToken();
            SourceLocation location = makeSourceLocation(nToken);
            const String  &name     = srcManager.GetSub(nToken.index, nToken.length);

            TypeExpr *type = nullptr;
            if (match(TokenType::Colon))
            {
                auto result = parseTypeExpr();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                type = *result;
            }

            Expr *defaultValue = nullptr;

            if (match(TokenType::Assign))
            {
                SET_STOP_AT(TokenType::Comma, TokenType::RightParen, TokenType::LeftBrace);
                auto result = parseExpression();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                defaultValue = *result;
            }

            PosParam *posParam = arena.Allocate<PosParam>(name, type, defaultValue, location);
            params.push_back(posParam);

            if (match(TokenType::Comma))
            {
                if (!currentToken().isIdentifier())
                {
                    return std::unexpected(
                        makeUnexpectTokenError("fn params", "param name", currentToken()));
                }
            }
        }
        return params;
    }

    Result<FnDefStmt *, Error> Parser::parseFnDefStmt(bool isPublic)
    {
        SourceLocation location = makeSourceLocation(consumeToken());

        if (!currentToken().isIdentifier())
        {
            return std::unexpected(
                makeUnexpectTokenError("fn def stmt", "function name", currentToken()));
        }
        const Token  &nameToken = consumeToken();
        const String &name      = srcManager.GetSub(nameToken.index, nameToken.length);

        if (currentToken().type != TokenType::LeftParen)
        {
            return std::unexpected(
                makeUnexpectTokenError("fn def stmt", "lparen '('", currentToken()));
        }

        DynArray<Param *> params;

        auto paraResult = parseFnParams();
        if (!paraResult)
        {
            return std::unexpected(paraResult.error());
        }
        params = *paraResult;

        TypeExpr *returnType = nullptr;
        if (match(TokenType::RightArrow))
        {
            auto result = parseTypeExpr();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            returnType = *result;
        }

        if (currentToken().type != TokenType::LeftBrace)
        {
            return std::unexpected(
                makeUnexpectTokenError("fn def stmt", "function body '{'", currentToken()));
        }
        BlockStmt *body       = nullptr;
        auto       bodyResult = parseBlockStmt();

        if (!bodyResult)
        {
            return std::unexpected(bodyResult.error());
        }
        body = *bodyResult;

        FnDefStmt *fnDef =
            arena.Allocate<FnDefStmt>(isPublic, name, params, returnType, body, location);
        return fnDef;
    }

    Result<ReturnStmt *, Error> Parser::parseReturnStmt()
    {
        StateProtector p(this, {State::ParsingReturn});

        SourceLocation location = makeSourceLocation(consumeToken());
        auto           result   = parseExpression();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        Expr       *value      = *result;
        ReturnStmt *returnStmt = arena.Allocate<ReturnStmt>(value, location);

        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }
        return returnStmt;
    }

    Result<Stmt *, Error> Parser::parseStatement()
    {
        StateProtector p(this, {State::Standby});

        if (currentToken().type == TokenType::Public)
        {
            consumeToken();
            if (currentToken().type == TokenType::Variable)
            {
                return parseVarDecl(true);
            }

            if (currentToken().type == TokenType::Function)
            {
                return parseFnDefStmt(true);
            }

            return std::unexpected(
                makeUnexpectTokenError("public", "var/const/func/struct", currentToken()));
        }

        if (currentToken().type == TokenType::LeftBrace)
        {
            return parseBlockStmt();
        }

        if (currentToken().type == TokenType::Variable)
        {
            return parseVarDecl(false);
        }

        if (currentToken().type == TokenType::If)
        {
            return parseIfStmt();
        }

        if (currentToken().type == TokenType::While)
        {
            return parseWhileStmt();
        }

        if (currentToken().type == TokenType::Function)
        {
            return parseFnDefStmt(false);
        }

        if (currentToken().type == TokenType::Return)
        {
            return parseReturnStmt();
        }

        if (match(TokenType::Break))
        {
            SourceLocation location = makeSourceLocation(prevToken());
            if (!match(TokenType::Semicolon))
            {
                return std::unexpected(makeExpectSemicolonError());
            }
            BreakStmt *breakStmt = arena.Allocate<BreakStmt>(location);
            return breakStmt;
        }

        if (match(TokenType::Continue))
        {
            SourceLocation location = makeSourceLocation(prevToken());
            if (!match(TokenType::Semicolon))
            {
                return std::unexpected(makeExpectSemicolonError());
            }
            ContinueStmt *continueStmt = arena.Allocate<ContinueStmt>(location);
            return continueStmt;
        }

        if (isEOF)
        {
            return nullptr;
        }

        const auto &expr_result = parseExpression();
        if (!expr_result)
        {
            return std::unexpected(expr_result.error());
        }
        ExprStmt *exprStmt = arena.Allocate<ExprStmt>(*expr_result);
        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }
        return exprStmt;
    }
}; // namespace Fig