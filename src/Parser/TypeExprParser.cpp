/*!
    @file src/Parser/TypeExprParser.cpp
    @brief 类型表达式解析器实现：支持泛型与空安全
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<decltype(StructDefStmt::typeParameters), Error> Parser::parseTypeParameters()
    {
        StateProtector                          p(this, {State::ParsingTypeParameters});
        decltype(StructDefStmt::typeParameters) tp;

        const Token &lab = consumeToken(); // consume `<`

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "unclosed `<` in type parameters",
                    "insert '>'",
                    makeSourceLocation(lab)));
            }
            if (match(TokenType::Greater)) // >
            {
                break;
            }
            if (!currentToken().isIdentifier())
            {
                return std::unexpected(
                    makeUnexpectTokenError("TypeParams", "tp name", currentToken()));
            }

            const Token  &name_tok = consumeToken();
            const String &name     = srcManager.GetSub(name_tok.index, name_tok.length);
            tp.push_back(name);

            if (!match(TokenType::Comma))
            {
                return std::unexpected(makeUnexpectTokenError(
                    "TypeParams", "comma or type parameter", currentToken()));
            }
        }
        return tp;
    }

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
                    return std::unexpected(
                        makeUnexpectTokenError("Type", "identifier", currentToken()));
            }
            else
                break;
        }

        DynArray<TypeExpr *> arguments;
        if (match(TokenType::Less)) // `<`
        {
            while (true)
            {
                auto result = parseTypeExpr();
                if (!result)
                    return std::unexpected(result.error());
                arguments.push_back(*result);

                if (match(TokenType::Greater))
                    break; // `>`
                if (!match(TokenType::Comma))
                    return std::unexpected(
                        makeUnexpectTokenError("TypeArgs", "'>' or ','", currentToken()));
            }
        }

        return arena.Allocate<NamedTypeExpr>(path, arguments, location);
    }

    Result<TypeExpr *, Error> Parser::parseFnTypeExpr()
    {
        StateProtector p(this, {State::ParsingFnTypeExpr});
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `func`
        if (!match(TokenType::LeftParen))                             // `(`
        {
            return std::unexpected(
                makeUnexpectTokenError("FnTypeExpr", "lparen (", currentToken()));
        }

        DynArray<TypeExpr *> paraTypes;

        while (true)
        {
            auto result = parseTypeExpr();
            if (!result)
            {
                return result;
            }
            paraTypes.push_back(*result);

            if (match(TokenType::RightParen))
            {
                break;
            }
            else if (isEOF)
            {
                return std::unexpected(
                    makeUnexpectTokenError("FnTypeExpr", "rparen )", currentToken()));
            }
            if (!match(TokenType::Comma))
            {
                return std::unexpected(
                    makeUnexpectTokenError("FnTypeExpr", "comma ,", currentToken()));
            }
        }

        TypeExpr *returnType = nullptr;

        if (match(TokenType::RightArrow)) // ->
        {
            auto result = parseTypeExpr();
            if (!result)
            {
                return result;
            }
            returnType = *result;
        }

        FnTypeExpr *fnTypeExpr = arena.Allocate<FnTypeExpr>(paraTypes, returnType);
        return fnTypeExpr;
    }

    // 解析主入口: 处理 `?` 后缀
    Result<TypeExpr *, Error> Parser::parseTypeExpr()
    {
        TypeExpr *base = nullptr;

        if (currentToken().isIdentifier())
        {
            auto result = parseNamedTypeExpr();
            if (!result)
            {
                return result;
            }
            base = *result;
        }
        else if (currentToken().type == TokenType::Function)
        {
            auto result = parseFnTypeExpr();
            if (!result)
            {
                return result;
            }
            base = *result;
        }
        else
        {
            return std::unexpected(makeUnexpectTokenError("TypeExpr", "name", currentToken()));
        }

        // type (?)
        if (currentToken().type == TokenType::Question) // ?
        {
            base = arena.Allocate<NullableTypeExpr>(
                base, makeSourceLocation(consumeToken())); // consume `?`
        }

        return base;
    }
} // namespace Fig
