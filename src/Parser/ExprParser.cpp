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
        LiteralExpr *node          = new LiteralExpr(literal_token, makeSourcelocation(literal_token));
        return node;
    }
    Result<IdentiExpr *, Error> Parser::parseIdentiExpr() // 当前token为Identifier调用
    {
        state                   = State::ParsingIdentiExpr;
        const Token &identifier = consumeToken();
        IdentiExpr  *node =
            new IdentiExpr(srcManager.GetSub(identifier.index, identifier.length), makeSourcelocation(identifier));
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

    std::unordered_set<TokenType> Parser::getTerminators() // 返回当前state的终止条件(终止符)
    {
        using enum State;

        static const std::unordered_set<TokenType> baseTerminators = {TokenType::EndOfFile, TokenType::Semicolon};

        switch (state)
        {
            default: return baseTerminators;
        }
    }
    bool Parser::shouldTerminate()
    {
        const Token &token       = currentToken();
        const auto  &terminators = getTerminators();
        return terminators.contains(token.type);
    }

    Result<Expr *, Error> Parser::parseExpression(BindingPower rbp)
    {
        Expr *lhs   = nullptr;
        Token token = currentToken();

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
        else if (IsTokenOp(token.type, false)) // 是否是一元运算符
        {
            const auto &lhs_result = parsePrefixExpr();
            if (!lhs_result)
            {
                return std::unexpected(lhs_result.error());
            }
            lhs = *lhs_result;
        }

        if (!lhs)
        {
            return std::unexpected(Error(ErrorType::ExpectedExpression,
                "expected expression",
                "insert expressions",
                makeSourcelocation(prevToken())));
        }

        while (true)
        {
            token = currentToken();
            if (shouldTerminate())
            {
                return lhs;
            }

            if (IsTokenOp(token.type /* isBinary = true */)) // 是否为二元运算符
            {
                BinaryOperator op  = TokenToBinaryOp(token);
                BindingPower   lbp = GetBinaryOpLBp(op);
                if (rbp >= lbp)
                {
                    // 前操作数的右绑定力比当前操作数的左绑定力大
                    // lhs被吸走
                    return lhs;
                }

                const auto &result = parseInfixExpr(lhs);
                if (!result)
                {
                    return result;
                }
                lhs = *result;
            }
            // 后缀运算符优先级非常大，几乎永远跟在操作数后面，因此我们可以直接结合
            // 而不用走正常路径
            else if (0) {}
            else
            {
                return lhs;
            }
        }
    }

}; // namespace Fig