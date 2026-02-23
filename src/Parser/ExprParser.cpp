/*!
    @file src/Parser/ExprParser.hpp
    @brief 语法分析器(Pratt + 手动递归下降) 表达式解析实现 (pratt)
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<LiteralExpr *, Error> Parser::parseLiteralExpr() // 当前token为literal时调用
    {
        state                      = State::ParsingLiteralExpr;
        const Token &literal_token = consumeToken();
        LiteralExpr *node = new LiteralExpr(literal_token, makeSourceLocation(literal_token));
        return node;
    }
    Result<IdentiExpr *, Error> Parser::parseIdentiExpr() // 当前token为Identifier调用
    {
        state                   = State::ParsingIdentiExpr;
        const Token &identifier = consumeToken();
        IdentiExpr  *node       = new IdentiExpr(
            srcManager.GetSub(identifier.index, identifier.length), makeSourceLocation(identifier));
        return node;
    }

    Result<InfixExpr *, Error> Parser::parseInfixExpr(Expr *lhs) // 当前token为 op
    {
        state                   = State::ParsingInfixExpr;
        const Token   &op_token = consumeToken();
        BinaryOperator op       = TokenToBinaryOp(op_token);
        BindingPower   rbp      = GetBinaryOpRBp(op);

        const auto &rhs_result = parseExpression(rbp);
        if (!rhs_result)
        {
            return std::unexpected(rhs_result.error());
        }
        Expr *rhs = *rhs_result;

        InfixExpr *node = new InfixExpr(lhs, op, rhs);
        return node;
    }

    Result<PrefixExpr *, Error> Parser::parsePrefixExpr() // 当前token为op
    {
        state                  = State::ParsingPrefixExpr;
        const Token  &op_token = consumeToken();
        UnaryOperator op       = TokenToUnaryOp(op_token);

        BindingPower rbp        = GetUnaryOpRBp(op);
        const auto  &rhs_result = parseExpression(rbp);
        if (!rhs_result)
        {
            return std::unexpected(rhs_result.error());
        }

        Expr       *rhs  = *rhs_result;
        PrefixExpr *node = new PrefixExpr(op, rhs);
        return node;
    }

    Result<IndexExpr *, Error> Parser::parseIndexExpr(
        Expr *base) // 由 parseExpression调用, 当前token为 `[`
    {
        state                       = State::ParsingIndexExpr;
        const Token &lbracket_token = consumeToken(); // consume `[`
        const auto  &index_result   = parseExpression();

        if (!index_result)
        {
            return std::unexpected(index_result.error());
        }

        if (currentToken().type != TokenType::RightBracket) // `]`
        {
            return std::unexpected(Error(ErrorType::SyntaxError,
                "unclosed brackets",
                "insert `]`",
                makeSourceLocation(lbracket_token)));
        }
        consumeToken(); // consume `]`

        IndexExpr *indexExpr = new IndexExpr(base, *index_result);
        return indexExpr;
    }

    Result<CallExpr *, Error> Parser::parseCallExpr(
        Expr *callee) // 由 parseExpression调用, 当前token为 `(`
    {
        state                     = State::ParsingCallExpr;
        const Token &lparen_token = consumeToken(); // consume `(`

        FnCallArgs callArgs;

        // 空参数列表
        if (currentToken().type == TokenType::RightParen)
        {
            consumeToken(); // consume `)`
            return new CallExpr(callee, callArgs);
        }

        while (true)
        {
            if (currentToken().type == TokenType::EndOfFile)
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
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
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "expected `,` or `)` in argument list",
                    "insert `,`",
                    makeSourceLocation(currentToken())));
            }

            consumeToken(); // consume `,`
        }

        return new CallExpr(callee, callArgs);
    }

    const std::unordered_set<TokenType> &Parser::getBaseTerminators()
    {
        static const std::unordered_set<TokenType> baseTerminators = {TokenType::Semicolon,
            TokenType::RightParen,
            TokenType::RightBracket,
            TokenType::RightBrace,
            TokenType::Comma,
            TokenType::EndOfFile};
        return baseTerminators;
    }

    std::unordered_set<TokenType> &Parser::getTerminators()
    {
        /*

        Syntax terminators:
            ;  )  ]  }  ,  EOF
        */

        static std::unordered_set<TokenType> terminators(getBaseTerminators());
        return terminators;
    }

    void Parser::resetTermintors()
    {
        getTerminators() = getBaseTerminators();
    }

    bool Parser::shouldTerminate()
    {
        const Token &token       = currentToken();
        const auto  &terminators = getTerminators();
        return terminators.contains(token.type);
    }

    Result<Expr *, Error> Parser::parseExpression(BindingPower rbp, TokenType stop, TokenType stop2)
    {
        if (!getTerminators().contains(stop))
        {
            getTerminators().insert(stop);
        }
        if (!getTerminators().contains(stop2))
        {
            getTerminators().insert(stop2);
        }

        Expr *lhs   = nullptr;
        Token token = currentToken();

        if (token.isIdentifier())
        {
            const auto &lhs_result = parseIdentiExpr();
            if (!lhs_result)
            {
                resetTermintors();
                return std::unexpected(lhs_result.error());
            }
            lhs = *lhs_result;
        }
        else if (token.isLiteral())
        {
            const auto &lhs_result = parseLiteralExpr();
            if (!lhs_result)
            {
                resetTermintors();
                return std::unexpected(lhs_result.error());
            }
            lhs = *lhs_result;
        }
        else if (IsTokenOp(token.type, false)) // 是否是一元运算符
        {
            const auto &lhs_result = parsePrefixExpr();
            if (!lhs_result)
            {
                resetTermintors();
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
                resetTermintors();
                return expr_result;
            }
            const Token &rparen_token = consumeToken(); // consume `)`
            if (rparen_token.type != TokenType::RightParen)
            {
                return std::unexpected(Error(ErrorType::SyntaxError,
                    "unclosed parenthese",
                    "insert `)`",
                    makeSourceLocation(lparen_token)));
            }
            lhs = *expr_result;
        }

        if (!lhs)
        {
            resetTermintors();
            return std::unexpected(Error(ErrorType::ExpectedExpression,
                "expected expression",
                "insert expressions",
                makeSourceLocation(prevToken())));
        }

        while (true)
        {
            token = currentToken();
            if (shouldTerminate())
            {
                break;
            }
            if (token.type == stop || token.type == stop2)
            {
                break;
            }

            if (IsTokenOp(token.type /* isBinary = true */)) // 是否为二元运算符
            {
                BinaryOperator op  = TokenToBinaryOp(token);
                BindingPower   lbp = GetBinaryOpLBp(op);
                if (rbp >= lbp)
                {
                    // 前操作数的右绑定力比当前操作数的左绑定力大
                    // lhs被吸走
                    break;
                }

                const auto &result = parseInfixExpr(lhs);
                if (!result)
                {
                    resetTermintors();
                    return result;
                }
                lhs = *result;
            }
            // 后缀运算符优先级非常大，几乎永远跟在操作数后面，因此我们可以直接结合
            // 而不用走正常路径
            else if (token.type == TokenType::LeftBracket) // `[`
            {
                const auto &expr_result = parseIndexExpr(lhs);
                if (!expr_result)
                {
                    resetTermintors();
                    return expr_result;
                }
                lhs = *expr_result;
            }
            else if (token.type == TokenType::LeftParen) // `(`
            {
                const auto &expr_result = parseCallExpr(lhs);
                if (!expr_result)
                {
                    resetTermintors();
                    return expr_result;
                }
                lhs = *expr_result;
            }
            else
            {
                return std::unexpected(Error(ErrorType::ExpectedExpression,
                    "expression unexpectedly ended",
                    "insert expressions",
                    makeSourceLocation(token)));
            }
        }
        return lhs;
    }

}; // namespace Fig