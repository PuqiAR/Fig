/*!
    @file src/Parser/ExprParser.hpp
    @brief 语法分析器(Pratt + 手动递归下降) 表达式解析实现 (pratt)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<Expr *, Error> Parser::parseLiteralExpr() // 当前token为literal时调用
    {
        StateProtector p(this, {State::ParsingLiteralExpr});

        const Token &literal_token = consumeToken();
        LiteralExpr *node =
            arena.Allocate<LiteralExpr>(literal_token, makeSourceLocation(literal_token));
        return node;
    }
    Result<Expr *, Error> Parser::parseIdentiExpr() // 当前token为Identifier调用
    {
        StateProtector p(this, {State::ParsingIdentiExpr});

        const Token &identifier = consumeToken();
        IdentiExpr  *node       = arena.Allocate<IdentiExpr>(
            srcManager.GetSub(identifier.index, identifier.length), makeSourceLocation(identifier));
        return node;
    }

    Result<Expr *, Error> Parser::parseInfixExpr(Expr *lhs) // 当前token为 op
    {
        StateProtector p(this, {State::ParsingInfixExpr});

        const Token   &op_token = consumeToken();
        BinaryOperator op       = TokenToBinaryOp(op_token);
        BindingPower   rbp      = GetBinaryOpRBp(op);

        const auto &rhs_result = parseExpression(rbp);
        if (!rhs_result)
        {
            return std::unexpected(rhs_result.error());
        }
        Expr *rhs = *rhs_result;

        InfixExpr *node = arena.Allocate<InfixExpr>(lhs, op, rhs);
        return node;
    }

    Result<Expr *, Error> Parser::parsePrefixExpr() // 当前token为op
    {
        StateProtector p(this, {State::ParsingPrefixExpr});

        const Token  &op_token = consumeToken();
        UnaryOperator op       = TokenToUnaryOp(op_token);

        BindingPower rbp        = GetUnaryOpRBp(op);
        const auto  &rhs_result = parseExpression(rbp);
        if (!rhs_result)
        {
            return std::unexpected(rhs_result.error());
        }

        Expr       *rhs  = *rhs_result;
        PrefixExpr *node = arena.Allocate<PrefixExpr>(op, rhs);
        return node;
    }

    Result<Expr *, Error>
    Parser::parseIndexExpr(Expr *base) // 由 parseExpression调用, 当前token为 `[`
    {
        StateProtector p(this, {State::ParsingIndexExpr});

        const Token &lbracket_token = consumeToken(); // consume `[`
        const auto  &index_result   = parseExpression();

        if (!index_result)
        {
            return std::unexpected(index_result.error());
        }

        if (currentToken().type != TokenType::RightBracket) // `]`
        {
            return std::unexpected(Error(
                ErrorType::SyntaxError,
                "unclosed brackets",
                "insert `]`",
                makeSourceLocation(lbracket_token)));
        }
        consumeToken(); // consume `]`

        IndexExpr *indexExpr = arena.Allocate<IndexExpr>(base, *index_result);
        return indexExpr;
    }

    Result<Expr *, Error>
    Parser::parseCallExpr(Expr *callee) // 由 parseExpression调用, 当前token为 `(`
    {
        StateProtector p(this, {State::ParsingCallExpr});

        const Token &lparen_token = consumeToken(); // consume `(`
        const SourceLocation &location = makeSourceLocation(lparen_token);

        FnCallArgs callArgs;

        // 空参数列表
        if (currentToken().type == TokenType::RightParen)
        {
            consumeToken(); // consume `)`
            return arena.Allocate<CallExpr>(callee, callArgs, location);
        }

        while (true)
        {
            if (currentToken().type == TokenType::EndOfFile)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "fn call has unclosed parenthese",
                    "insert `)`",
                    makeSourceLocation(lparen_token)));
            }

            const auto &arg_result = parseExpression();
            if (!arg_result)
                return std::unexpected(arg_result.error());

            callArgs.args.push_back(*arg_result);

            if (currentToken().type == TokenType::RightParen)
            {
                consumeToken(); // consume `)`
                break;
            }

            if (currentToken().type != TokenType::Comma)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "expected `,` or `)` in argument list",
                    "insert `,`",
                    makeSourceLocation(currentToken())));
            }

            consumeToken(); // consume `,`
        }

        return arena.Allocate<CallExpr>(callee, callArgs, location);
    }

    Result<Expr *, Error> Parser::parseNewExpr()
    {
        // new type{...}
        StateProtector p(this, {State::ParsingNewExpr});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `new`

        SET_STOP_AT(TokenType::LeftBrace); // {
        auto type_result = parseTypeExpr();
        if (!type_result)
        {
            return std::unexpected(type_result.error());
        }
        Expr *type = *type_result;

        if (!match(TokenType::LeftBrace))
        {
            return std::unexpected(makeUnexpectTokenError("NewExpr", "lbrace {", currentToken()));
        }

        const Token &lb_token = prevToken();

        /*
            Positional:
                new Point{1, 2}
            Named:
                new Point{x = 1, y = 2}
            Shorthand:
                new Point{y, x}
        */

        DynArray<NewExpr::Arg> args;

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "unclosed `{` in new expr",
                    "insert '}'",
                    makeSourceLocation(lb_token)
                ));
            }
            if (args.empty() && match(TokenType::RightBrace)) // 空参
            {
                break;
            }

            // named arg
            if (currentToken().isIdentifier() && peekToken().type == TokenType::Colon)
            {
                const Token &name_token = consumeToken();
                const String &name = srcManager.GetSub(name_token.index, name_token.length);
                consumeToken(); // consume `:`

                SET_STOP_AT(TokenType::Comma, TokenType::RightBrace); // , / }
                auto result = parseExpression();
                if (!result)
                {
                    return result;
                }

                args.push_back(NewExpr::Arg{
                    name,
                    *result
                });
            }
            // shorthand
            else if (currentToken().isIdentifier()
                     && (peekToken().type == TokenType::Comma || peekToken().type == TokenType::RightBrace))
            {
                const Token &name_token = consumeToken();
                const String &name = srcManager.GetSub(name_token.index, name_token.length);

                
                
                IdentiExpr *ident =
                    arena.Allocate<IdentiExpr>(name, makeSourceLocation(name_token));
                args.push_back(NewExpr::Arg{name, ident});
            }
            else
            {
                SET_STOP_AT(TokenType::Comma, TokenType::RightBrace); // , / }
                auto result = parseExpression();
                if (!result)
                {
                    return result;
                }

                args.push_back(NewExpr::Arg{
                    .value = *result
                });
            }


            if (match(TokenType::Comma))
            {
                continue;
            }

            if (match(TokenType::RightBrace))
            {
                break;
            }
        }

        NewExpr *newExpr = arena.Allocate<NewExpr>(type, args, location);
        return newExpr;
    }

    Result<Expr *, Error> Parser::parseLambdaExpr()
    {
        StateProtector p(this, {State::ParsingLambdaExpr});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `func`

        if (currentToken().isIdentifier())
        {
            return std::unexpected(Error(
                ErrorType::SyntaxError,
                "lambda expression should not have a name",
                "remove the name",
                makeSourceLocation(currentToken())));
        }

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

        Expr *returnType = nullptr;
        Token     rightArrowToken;
        if (match(TokenType::RightArrow)) // ->
        {
            rightArrowToken = consumeToken();

            auto result = parseTypeExpr();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            returnType = *result;
        }

        if (match(TokenType::DoubleArrow)) // =>
        {
            if (returnType)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "use of expr body but specified return type in lambda expr",
                    "remove `-> ...`",
                    makeSourceLocation(rightArrowToken)));
            }
            auto result = parseExpression();
            if (!result)
            {
                return result;
            }

            Expr       *expr = *result;
            LambdaExpr *lambda =
                arena.Allocate<LambdaExpr>(params, returnType, expr, true, location);
            return lambda;
        }
        else if (currentToken().type == TokenType::LeftBrace)
        {
            auto result = parseBlockStmt();
            if (!result)
            {
                return std::unexpected(result.error());
            }

            LambdaExpr *lambda =
                arena.Allocate<LambdaExpr>(params, returnType, *result, false, location);
            return lambda;
        }
        else
        {
            return std::unexpected(
                makeUnexpectTokenError("LambdaExpr", "darrow => / lbrace {", currentToken()));
        }
    }

    Result<Expr *, Error> Parser::parseExpression(BindingPower rbp)
    {
        Expr *lhs   = nullptr;
        Token token = currentToken();

        // NUD
        if (token.isIdentifier())
        {
            const auto &lhs_result = parseIdentiExpr();
            if (!lhs_result)
            {
                return std::unexpected(lhs_result.error());
            }
            lhs = *lhs_result;
        }
        else if (token.isLiteral())
        {
            const auto &lhs_result = parseLiteralExpr();
            if (!lhs_result)
            {
                return std::unexpected(lhs_result.error());
            }
            lhs = *lhs_result;
        }
        else if (IsTokenOp(token.type, false)) // 是否是一元前缀运算符
        {
            const auto &lhs_result = parsePrefixExpr();
            if (!lhs_result)
            {
                return std::unexpected(lhs_result.error());
            }
            lhs = *lhs_result;
        }
        else if (token.type == TokenType::LeftParen)
        {
            const Token &lparen_token = consumeToken(); // consume `(`
            const auto  &expr_result  = parseExpression(0);
            if (!expr_result)
            {
                return expr_result;
            }
            const Token &rparen_token = consumeToken(); // consume `)`
            if (rparen_token.type != TokenType::RightParen)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "unclosed parenthese",
                    "insert `)`",
                    makeSourceLocation(lparen_token)));
            }
            lhs = *expr_result;
        }
        else if (token.type == TokenType::Function)
        {
            auto result = parseLambdaExpr();
            if (!result)
            {
                return result;
            }

            lhs = *result;
        }
        else if (token.type == TokenType::New)
        {
            auto result = parseNewExpr();
            if (!result)
            {
                return result;
            }

            lhs = *result;
        }

        if (!lhs)
        {
            return std::unexpected(Error(
                ErrorType::ExpectedExpression,
                "expected expression",
                "insert expressions",
                makeSourceLocation(prevToken())));
        }

        // LED
        while (true)
        {
            token = currentToken();
            if (shouldTerminate())
            {
                break;
            }

            // is / as
            if (token.type == TokenType::Is || token.type == TokenType::As)
            {
                BinaryOperator op  = TokenToBinaryOp(token);
                BindingPower   lbp = GetBinaryOpLBp(op);
                if (rbp >= lbp)
                {
                    break;
                }
                consumeToken(); // consume `is` or `as`
                auto typeRes = parseTypeExpr();
                if (!typeRes)
                {
                    return std::unexpected(typeRes.error());
                }
                lhs = arena.Allocate<InfixExpr>(lhs, op, *typeRes);
            }
            // binary
            else if (IsTokenOp(token.type /* isBinary = true */))
            {
                BinaryOperator op  = TokenToBinaryOp(token);
                BindingPower   lbp = GetBinaryOpLBp(op);
                if (rbp >= lbp)
                {
                    break;
                }

                auto result = parseInfixExpr(lhs);
                if (!result)
                {
                    return result;
                }
                lhs = *result;
            }
            // [index]
            else if (token.type == TokenType::LeftBracket)
            {
                const auto &expr_result = parseIndexExpr(lhs);
                if (!expr_result)
                {
                    return expr_result;
                }
                lhs = *expr_result;
            }
            // call
            else if (token.type == TokenType::LeftParen)
            {
                const auto &expr_result = parseCallExpr(lhs);
                if (!expr_result)
                {
                    return expr_result;
                }
                lhs = *expr_result;
            }
            // .member
            else if (token.type == TokenType::Dot)
            {
                consumeToken(); // consume `.`
                if (!currentToken().isIdentifier())
                {
                    return std::unexpected(
                        makeUnexpectTokenError("MemberExpr", "identifier after `.`", currentToken()));
                }
                const Token  &nameToken = consumeToken();
                const String &name =
                    srcManager.GetSub(nameToken.index, nameToken.length);
                SourceLocation loc = makeSourceLocation(nameToken);
                lhs = arena.Allocate<MemberExpr>(lhs, name, loc);
            }
            // x++ x--
            else if (token.type == TokenType::DoublePlus || token.type == TokenType::DoubleMinus)
            {
                UnaryOperator op = TokenToUnaryOp(consumeToken());
                lhs = arena.Allocate<PostfixExpr>(op, lhs);
            }
            // ?:
            else if (token.type == TokenType::Question)
            {
                // ?: 最低优先
                // 赋值 rbp = 101，所以只有当 rbp < 100 时才可能进到三元
                // 实际上三元是最低优先级的非赋值运算符，我们给一个很小的 lbp
                constexpr BindingPower TERNARY_LBP = 150;
                if (rbp >= TERNARY_LBP)
                {
                    break;
                }
                consumeToken(); // consume `?`
                auto thenRes = parseExpression(0); // 重置绑定力，右结合
                if (!thenRes)
                {
                    return std::unexpected(thenRes.error());
                }
                if (!match(TokenType::Colon))
                {
                    return std::unexpected(
                        makeUnexpectTokenError("TernaryExpr", "`:` for else branch", currentToken()));
                }
                auto elseRes = parseExpression(TERNARY_LBP - 1); // 右结合
                if (!elseRes)
                {
                    return std::unexpected(elseRes.error());
                }
                lhs = arena.Allocate<TernaryExpr>(lhs, *thenRes, *elseRes, lhs->location);
            }
            else
            {
                return lhs;
            }
        }
        return lhs;
    }

}; // namespace Fig
