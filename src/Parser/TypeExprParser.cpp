/*!
    @file src/Parser/TypeExprParser.cpp
    @brief 类型表达式解析器实现 — 类型即值，产生 Expr* 而非 TypeExpr*
    @author PuqiAR (im@puqiar.top)
    @date 2026-06-06
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

    
    
    Result<Expr *, Error> Parser::parseNamedTypeExpr()
    {
        StateProtector p(this, {State::ParsingNamedTypeExpr});

        
        if (!currentToken().isIdentifier())
        {
            return std::unexpected(
                makeUnexpectTokenError("TypeExpr", "type name", currentToken()));
        }

        const Token  &firstTok  = consumeToken();
        SourceLocation firstLoc = makeSourceLocation(firstTok);
        const String &firstName = srcManager.GetSub(firstTok.index, firstTok.length);

        IdentiExpr *ident = arena.Allocate<IdentiExpr>(firstName, firstLoc);
        Expr       *base  = ident;

        // a.b.c
        while (match(TokenType::Dot))
        {
            if (!currentToken().isIdentifier())
            {
                return std::unexpected(
                    makeUnexpectTokenError("TypeExpr", "identifier after `.`", currentToken()));
            }
            const Token  &tok  = consumeToken();
            const String &name = srcManager.GetSub(tok.index, tok.length);
            SourceLocation loc = makeSourceLocation(tok);
            base = arena.Allocate<MemberExpr>(base, name, loc);
        }

        // generic args
        if (match(TokenType::Less))
        {
            DynArray<Expr *> args;

            while (true)
            {
                auto result = parseTypeExpr();
                if (!result)
                    return std::unexpected(result.error());
                args.push_back(*result);

                if (match(TokenType::Greater))
                    break; // `>`
                if (!match(TokenType::Comma))
                    return std::unexpected(
                        makeUnexpectTokenError("TypeArgs", "'>' or ','", currentToken()));
            }

            base = arena.Allocate<ApplyExpr>(base, std::move(args), firstLoc);
        }

        return base;
    }

    Result<Expr *, Error> Parser::parseFnTypeExpr()
    {
        StateProtector p(this, {State::ParsingFnTypeExpr});
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `func`
        if (!match(TokenType::LeftParen))                             // `(`
        {
            return std::unexpected(
                makeUnexpectTokenError("FnTypeExpr", "lparen (", currentToken()));
        }

        DynArray<Expr *> paraTypes;

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

        Expr *returnType = nullptr;

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

    
    Result<Expr *, Error> Parser::parseTypeExpr()
    {
        Expr *base = nullptr;

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

        // nullable
        if (currentToken().type == TokenType::Question) // ?
        {
            base = arena.Allocate<NullableExpr>(
                base, makeSourceLocation(consumeToken())); // consume `?`
        }

        return base;
    }
} // namespace Fig
