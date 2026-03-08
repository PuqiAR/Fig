/*!
    @file src/Parser/TypeExprParser.cpp
    @brief 类型表达式解析器实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-25
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<NamedTypeExpr *, Error> Parser::parseNamedTypeExpr() // 当前token为identifier
    {
        StateProtector p(this, {State::ParsingNamedTypeExpr});

        SourceLocation location = makeSourceLocation(currentToken());

        DynArray<String> path;
        while (true)
        {
            const Token  &subPathTok = consumeToken();
            const String &subPath    = srcManager.GetSub(subPathTok.index, subPathTok.length);
            path.push_back(subPath);

            if (match(TokenType::Dot))
            {
                if (!currentToken().isIdentifier())
                {
                    return std::unexpected(
                        makeUnexpectTokenError("named type expr", "identifier", currentToken()));
                }
            }
            else 
            {
                break;
            }
        }
        NamedTypeExpr *namedTypeExpr = arena.Allocate<NamedTypeExpr>(path, location);
        return namedTypeExpr;
    }

    Result<TypeExpr *, Error> Parser::parseTypeExpr()
    {
        // TODO: 泛型表达式解析
        if (currentToken().isIdentifier())
        {
            return parseNamedTypeExpr();
        }
        else
        {
            return std::unexpected(makeUnexpectTokenError("type expr", "name/...", currentToken()));
        }
    }
}; // namespace Fig