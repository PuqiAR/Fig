/*!
    @file src/Parser/TypeExprParser.cpp
    @brief 类型表达式解析器实现：支持泛型与空安全
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    // 解析基础命名类型与泛型: List<Int>
    Result<TypeExpr *, Error> Parser::parseNamedTypeExpr()
    {
        StateProtector p(this, {State::ParsingNamedTypeExpr});
        SourceLocation location = makeSourceLocation(currentToken());

        DynArray<String> path;
        while (true)
        {
            const Token  &tok  = consumeToken();
            const String &name = srcManager.GetSub(tok.index, tok.length);
            path.push_back(name);

            if (match(TokenType::Dot))
            {
                if (!currentToken().isIdentifier())
                    return std::unexpected(makeUnexpectTokenError("Type", "identifier", currentToken()));
            }
            else break;
        }

        DynArray<TypeExpr *> arguments;
        if (match(TokenType::Less)) // `<`
        {
            while (true)
            {
                auto result = parseTypeExpr();
                if (!result) return std::unexpected(result.error());
                arguments.push_back(*result);

                if (match(TokenType::Greater)) break; // `>`
                if (!match(TokenType::Comma))
                    return std::unexpected(makeUnexpectTokenError("TypeArgs", "'>' or ','", currentToken()));
            }
        }

        return arena.Allocate<NamedTypeExpr>(path, arguments, location);
    }

    // 解析主入口: 处理 `?` 后缀
    Result<TypeExpr *, Error> Parser::parseTypeExpr()
    {
        TypeExpr *base = nullptr;

        // 目前只支持命名类型 (以后可以加函数类型 (Int)->Int)
        if (currentToken().isIdentifier())
        {
            auto res = parseNamedTypeExpr();
            if (!res) return std::unexpected(res.error());
            base = *res;
        }
        else return std::unexpected(makeUnexpectTokenError("TypeExpr", "name", currentToken()));

        // 空安全处理: Int?? 也可以，但 Analyzer 会规范化它
        while (match(TokenType::Question))
        {
            base = arena.Allocate<NullableTypeExpr>(base, makeSourceLocation(prevToken()));
        }

        return base;
    }
}
