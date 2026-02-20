/*!
    @file src/Parser/StmtParser.hpp
    @brief 语法分析器(Pratt + 手动递归下降) 语句解析实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<VarDecl *, Error> Parser::parseVarDecl(bool isPublic) // 由 parseStatement调用, 当前token为 var
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
    Result<Stmt *, Error> Parser::parseStatement()
    {
        if (currentToken().type == TokenType::Public)
        {
            consumeToken(); // consume `public`
            if (currentToken().type == TokenType::Variable)
            {
                return parseVarDecl(true);
            }
        }
        else if (currentToken().type == TokenType::Variable)
        {
            return parseVarDecl(false);
        }
        else
        {
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
    }
}; // namespace Fig