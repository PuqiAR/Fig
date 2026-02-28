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
    Result<VarDecl *, Error> Parser::parseVarDecl(
        bool isPublic) // 由 parseStatement调用, 当前token为 var
    {
        StateProtector p(this, {State::ParsingVarDecl});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `var`

        if (currentToken().type != TokenType::Identifier)
        {
            return std::unexpected(makeUnexpectTokenError("VarDecl", "var name", currentToken()));
        }
        const String &name = srcManager.GetSub(currentToken().index, currentToken().length);
        consumeToken(); // consume name

        TypeExpr *typeSpeicifer = nullptr;
        if (match(TokenType::Colon)) // `:`
        {
            // SET_STOP_AT(TokenType::Walrus, TokenType::Assign);
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
        else if (match(TokenType::Walrus)) // :=
        {
            if (typeSpeicifer) // 指定了类型同时使用 :=
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "used type infer but specifying the type",
                    "change `:=` to '='",
                    makeSourceLocation(prevToken()) // :=
                    ));
            }
            auto result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            initExpr = *result;
            isInfer  = true; // 使用类型自动推断 :=
        }
        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }
        VarDecl *varDecl = new VarDecl(isPublic, name, typeSpeicifer, isInfer, initExpr, location);
        return varDecl;
    }

    Result<IfStmt *, Error> Parser::parseIfStmt() // 由 parseStatement调用, 当前token is if
    {
        StateProtector p(this, {State::ParsingIf});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `if`

        Expr *cond = nullptr;
        if (match(TokenType::LeftParen)) // match and consume `(`
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
                delete *result;
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

                    SET_STOP_AT(TokenType::RightParen, TokenType::LeftBrace);
                    const auto &result = parseExpression(0);
                    if (!result)
                    {
                        return std::unexpected(result.error());
                    }
                    if (!match(TokenType::RightParen))
                    {
                        delete *result;
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
                auto result = parseBlockStmt();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                alternate = *result;
            }
        }
        IfStmt *ifStmt = new IfStmt(cond, consequent, elifs, alternate, location);
        return ifStmt;
    }

    Result<WhileStmt *, Error> Parser::parseWhileStmt() // 由 parseStatement调用, 当前token为 while
    {
        StateProtector p(this, {State::ParsingWhile});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `while`

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
                delete *result;
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
            delete cond;
            return std::unexpected(
                makeUnexpectTokenError("while stmt", "left brace '{'", currentToken()));
        }

        auto result = parseBlockStmt();
        if (!result)
        {
            delete cond;
            return std::unexpected(result.error());
        }
        BlockStmt *body = *result;

        WhileStmt *whileStmt = new WhileStmt(cond, body, location);
        return whileStmt;
    }

    Result<DynArray<Param *>, Error> Parser::parseFnParams() // 由 parseFnDefStmt或lambda调用
    {
        StateProtector p(this, {State::ParsingFnDefStmt});

        const Token      &lpToken = consumeToken(); // consume `(`
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

            // TODO: 支持剩余参数解析...

            TypeExpr *type = nullptr;
            if (match(TokenType::Colon)) // :
            {
                auto result = parseTypeExpr();
                if (!result)
                {
                    return std::unexpected(result.error());
                }
                type = *result;
            }

            Expr *defaultValue = nullptr;

            if (match(TokenType::Assign)) // =
            {
                SET_STOP_AT(TokenType::Comma, TokenType::RightParen, TokenType::LeftBrace); // , ) {
                auto result = parseExpression();
                if (!result)
                {
                    if (type)
                    {
                        delete type;
                    }
                    return std::unexpected(result.error());
                }
                defaultValue = *result;
            }

            PosParam *posParam = new PosParam(name, type, defaultValue, location);
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

    Result<FnDefStmt *, Error> Parser::parseFnDefStmt(
        bool isPublic) // 由 parseStatement调用, 当前token为 func
    {
        SourceLocation location = makeSourceLocation(
            consumeToken()); // 无论是否加了public, location都设置为 func token (我懒 :D)

        if (!currentToken().isIdentifier())
        {
            return std::unexpected(
                makeUnexpectTokenError("fn def stmt", "function name", currentToken()));
        }
        const Token  &nameToken = consumeToken(); // consume name
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
        if (match(TokenType::RightArrow)) // ->
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
            if (returnType)
            {
                delete returnType;
            }
            return std::unexpected(bodyResult.error());
        }
        body = *bodyResult;

        FnDefStmt *fnDef = new FnDefStmt(isPublic, name, params, returnType, body, location);
        return fnDef;
    }

    Result<ReturnStmt *, Error>
    Parser::parseReturnStmt() // 由 parseStatement调用, 当前token为 return
    {
        StateProtector p(this, {State::ParsingReturn});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `return`
        auto           result   = parseExpression();
        if (!result)
        {
            return std::unexpected(result.error());
        }

        Expr       *value      = *result;
        ReturnStmt *returnStmt = new ReturnStmt(value, location);

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
            consumeToken(); // consume `public`
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
            BreakStmt *breakStmt = new BreakStmt(location);
            return breakStmt;
        }

        if (match(TokenType::Continue))
        {
            SourceLocation location = makeSourceLocation(prevToken());
            if (!match(TokenType::Semicolon))
            {
                return std::unexpected(makeExpectSemicolonError());
            }
            ContinueStmt *continueStmt = new ContinueStmt(location);
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
        ExprStmt *exprStmt = new ExprStmt(*expr_result);
        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }
        return exprStmt;
    }
}; // namespace Fig