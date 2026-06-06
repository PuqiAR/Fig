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
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
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

        Expr *typeSpeicifer = nullptr;
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
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
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

    Result<VarDecl *, Error> Parser::parseConstDecl(bool isPublic)
    {
        // must init
        StateProtector p(this, {State::ParsingVarDecl});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `const`

        if (currentToken().type != TokenType::Identifier)
        {
            return std::unexpected(makeUnexpectTokenError("ConstDecl", "const name", currentToken()));
        }
        const String &name = srcManager.GetSub(currentToken().index, currentToken().length);
        consumeToken();

        Expr *typeSpecifier = nullptr;
        if (match(TokenType::Colon))
        {
            auto result = parseTypeExpr();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            typeSpecifier = *result;
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
            if (typeSpecifier)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
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
        else
        {
            return std::unexpected(Error(
                ErrorType::SyntaxError,
                "const must be initialized",
                "add '=' and an initializer expression",
                makeSourceLocation(prevToken())));
        }

        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }

        VarDecl *varDecl =
            arena.Allocate<VarDecl>(isPublic, name, typeSpecifier, isInfer, initExpr, location);
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
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
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
                    return std::unexpected(Error(
                        ErrorType::SyntaxError,
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
                        return std::unexpected(Error(
                            ErrorType::SyntaxError,
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
                    return std::unexpected(Error(
                        ErrorType::SyntaxError,
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
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
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

    Result<Stmt *, Error> Parser::parseForStmt()
    {
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `for`

        // 括号可选
        bool hasParen = match(TokenType::LeftParen);

        // init: var decl 或 表达式语句（或空）
        Stmt *init = nullptr;
        if (currentToken().type == TokenType::Variable)
        {
            auto result = parseVarDecl(false);
            if (!result)
                return std::unexpected(result.error());
            init = *result;
        }
        else if (currentToken().type == TokenType::Semicolon)
        {
            // 空 init，跳过
        }
        else if (!isEOF)
        {
            // 表达式作为 init
            auto result = parseExpression();
            if (!result)
                return std::unexpected(result.error());
            init = arena.Allocate<ExprStmt>(*result);
            if (!match(TokenType::Semicolon))
                return std::unexpected(makeExpectSemicolonError());
        }

        // 要求分号分隔
        if (!init && currentToken().type != TokenType::Semicolon)
        {
            // 如果不是 var decl 且下一个不是分号，尝试作为表达式解析并消耗分号
            // 实际上 init 为 nullptr 的情况就是空 init，此时应该已经有分号了
        }
        if (init)
        {
            
        }
        else
        {
            if (!match(TokenType::Semicolon))
            {
                return std::unexpected(makeExpectSemicolonError());
            }
        }

        // cond: 表达式（或空）
        Expr *cond = nullptr;
        SET_STOP_AT(TokenType::Semicolon);
        if (currentToken().type != TokenType::Semicolon)
        {
            auto result = parseExpression();
            if (!result)
                return std::unexpected(result.error());
            cond = *result;
        }
        // 确保下一个是分号
        if (currentToken().type == TokenType::Semicolon)
        {
            // continue
        }
        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }

        // step: 表达式（或空）
        Expr *step = nullptr;
        if (hasParen)
        {
            SET_STOP_AT(TokenType::RightParen);
        }
        else
        {
            SET_STOP_AT(TokenType::LeftBrace);
        }
        if (hasParen && currentToken().type == TokenType::RightParen)
        {
            // 空 step
        }
        else if (!hasParen && currentToken().type == TokenType::LeftBrace)
        {
            // 空 step，直接进入 body
        }
        else if (!isEOF)
        {
            auto result = parseExpression();
            if (!result)
                return std::unexpected(result.error());
            step = *result;
        }

        if (hasParen && !match(TokenType::RightParen))
        {
            return std::unexpected(
                makeUnexpectTokenError("for stmt", "`)` to close", currentToken()));
        }

        if (currentToken().type != TokenType::LeftBrace)
        {
            return std::unexpected(
                makeUnexpectTokenError("for stmt", "left brace `{`", currentToken()));
        }

        auto bodyRes = parseBlockStmt();
        if (!bodyRes)
        {
            return std::unexpected(bodyRes.error());
        }

        ForStmt *forStmt = arena.Allocate<ForStmt>(init, cond, step, *bodyRes, location);
        return forStmt;
    }

    Result<DynArray<Param *>, Error> Parser::parseFnParams()
    {
        const Token      &lpToken = consumeToken();
        DynArray<Param *> params;

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
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

            Expr *type = nullptr;
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

            // 可变参数: ... (跟在最后一个参数后面)
            if (match(TokenType::TripleDot))
            {
                // 标记最后一个参数为可变参数
                // 通过 VarParam 标记
                // 当前用 VarParam 包装最后一个 param
                // 简单方案：创建一个带 variadic 标记的 param
                // TODO: 改用专门的 VarParam AST 节点
                // 暂时用 flag 标记在 posParam 上

                if (!match(TokenType::RightParen))
                {
                    return std::unexpected(
                        makeUnexpectTokenError("fn params", "`)` after `...`", currentToken()));
                }
                // 直接返回，可变参数后面不能再有参数
                break;
            }

            if (match(TokenType::Comma))
            {
                if (match(TokenType::RightParen))
                {
                    // 尾部逗号允许，如 func(a, b,)
                    break;
                }
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
        StateProtector p(this, {State::ParsingFnDefStmt});
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

        Expr *returnType = nullptr;
        if (match(TokenType::RightArrow))
        {
            auto result = parseTypeExpr();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            returnType = *result;
        }

        BlockStmt *body = nullptr;
        if (match(TokenType::DoubleArrow)) // =>
        {
            auto result = parseExpression();
            if (!result)
            {
                return std::unexpected(result.error());
            }

            if (match(TokenType::Semicolon))
            {
                diagnostics.Report(Error(
                    ErrorType::UnnecessarySemicolon,
                    "`;` is unnecessary in this context",
                    "try remove `;`",
                    makeSourceLocation(prevToken())));
            }

            Expr       *expr       = *result;
            ReturnStmt *returnStmt = arena.Allocate<ReturnStmt>(expr, expr->location);

            body = arena.Allocate<BlockStmt>();
            body->nodes.push_back(returnStmt);
        }
        else if (currentToken().type == TokenType::LeftBrace)
        {
            auto bodyResult = parseBlockStmt();
            if (!bodyResult)
            {
                return std::unexpected(bodyResult.error());
            }
            body = *bodyResult;
        }
        else
        {
            return std::unexpected(
                makeUnexpectTokenError("fn def stmt", "function body '=>' / '{'", currentToken()));
        }

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

    Result<Stmt *, Error> Parser::parseStructDef(bool isPublic)
    {
        StateProtector p(this, {State::ParsingStructDef});

        SourceLocation location = makeSourceLocation(consumeToken()); // consume `struct`
        if (!currentToken().isIdentifier())
        {
            return std::unexpected(
                makeUnexpectTokenError("StructDef", "struct name", currentToken()));
        }

        const Token  &name_tok = consumeToken(); // consume name
        const String &name     = srcManager.GetSub(name_tok.index, name_tok.length);

        StructDefStmt *stDef = arena.Allocate<StructDefStmt>();

        if (currentToken().type == TokenType::Less) // <
        {
            auto result = parseTypeParameters();
            if (!result)
            {
                return std::unexpected(result.error());
            }

            stDef->typeParameters = *result;
        }

        if (!match(TokenType::LeftBrace))
        {
            return std::unexpected(
                makeUnexpectTokenError("StructDef", "lbrace '{'", currentToken()));
        }

        const Token &lb_tok = prevToken(); // `{`

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "unclosed braces in struct def",
                    "insert '}'",
                    makeSourceLocation(lb_tok)));
            }
            if (match(TokenType::RightBrace))
            {
                break;
            }

            // (public) field_name (: Type) (= expr) / (:= expr)

            bool isPublicField = match(TokenType::Public);
            if (currentToken().isIdentifier())
            {
                const Token  &name_tok   = consumeToken();
                const String &field_name = srcManager.GetSub(name_tok.index, name_tok.length);

                if (match(TokenType::Walrus)) // :=
                {
                    auto result = parseExpression();
                    if (!result)
                    {
                        return std::unexpected(result.error());
                    }

                    stDef->fields.push_back(
                        StructDefStmt::Field{isPublicField, true, field_name, nullptr, *result});
                }
                else
                {
                    Expr *type     = nullptr;
                    Expr     *initExpr = nullptr;

                    if (match(TokenType::Colon)) // :
                    {
                        auto result = parseTypeExpr();
                        if (!result)
                        {
                            return std::unexpected(result.error());
                        }
                        type = *result;
                    }

                    if (match(TokenType::Assign))
                    {
                        auto result = parseExpression();
                        if (!result)
                        {
                            return std::unexpected(result.error());
                        }
                        initExpr = *result;
                    }
                    stDef->fields.push_back(
                        StructDefStmt::Field{isPublicField, false, field_name, type, initExpr});
                }
                if (!match(TokenType::Semicolon))
                {
                    return std::unexpected(makeExpectSemicolonError());
                }
            }
            else if (currentToken().type == TokenType::Function)
            {
                auto result = parseFnDefStmt(isPublicField);
                if (!result)
                {
                    return result;
                }

                stDef->methods.push_back(*result);
            }
            else
            {
                return std::unexpected(
                    makeUnexpectTokenError("StructDef", "field or method", currentToken()));
            }
        }

        return stDef;
    }

    Result<Stmt *, Error> Parser::parseInterfaceDef(bool isPublic)
    {
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `interface`

        if (!currentToken().isIdentifier())
        {
            return std::unexpected(
                makeUnexpectTokenError("InterfaceDef", "interface name", currentToken()));
        }

        const Token  &nameToken = consumeToken();
        const String &name      = srcManager.GetSub(nameToken.index, nameToken.length);

        if (!match(TokenType::LeftBrace))
        {
            return std::unexpected(
                makeUnexpectTokenError("InterfaceDef", "lbrace '{'", currentToken()));
        }

        const Token &lbToken = prevToken();

        InterfaceDefStmt *ifaceDef = arena.Allocate<InterfaceDefStmt>();
        ifaceDef->isPublic = isPublic;
        ifaceDef->name     = name;
        ifaceDef->location = location;

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "unclosed braces in interface def",
                    "insert '}'",
                    makeSourceLocation(lbToken)));
            }
            if (match(TokenType::RightBrace))
            {
                break;
            }

            // interface 方法需要 `func` 关键字
            if (currentToken().type != TokenType::Function)
            {
                return std::unexpected(
                    makeUnexpectTokenError("InterfaceDef", "`func` for method", currentToken()));
            }
            consumeToken(); // consume `func`

            if (!currentToken().isIdentifier())
            {
                return std::unexpected(
                    makeUnexpectTokenError("InterfaceDef", "method name", currentToken()));
            }

            const Token  &methodNameToken = consumeToken();
            const String &methodName =
                srcManager.GetSub(methodNameToken.index, methodNameToken.length);

            // 参数列表
            if (!match(TokenType::LeftParen))
            {
                return std::unexpected(
                    makeUnexpectTokenError("InterfaceDef", "`(` for method params", currentToken()));
            }

            // 解析参数类型（接口方法只声明类型，无参数名）
            DynArray<InterfaceDefStmt::Method> methods; // placeholder
            DynArray<Expr *> paramTypes;

            while (true)
            {
                if (isEOF)
                {
                    return std::unexpected(Error(
                        ErrorType::SyntaxError,
                        "unclosed parenthese in interface method",
                        "insert ')'",
                        makeSourceLocation(methodNameToken)));
                }
                if (match(TokenType::RightParen))
                {
                    break;
                }

                // 参数: name : Type
                if (!currentToken().isIdentifier())
                {
                    return std::unexpected(
                        makeUnexpectTokenError("InterfaceDef", "param name", currentToken()));
                }
                consumeToken(); // consume param name (unused in interface)

                Expr *paramType = nullptr;
                if (match(TokenType::Colon))
                {
                    auto res = parseTypeExpr();
                    if (!res)
                        return std::unexpected(res.error());
                    paramType = *res;
                }
                // 接口方法参数类型可选（如果没有标注则默认 Any）
                paramTypes.push_back(paramType);

                if (match(TokenType::Comma))
                {
                    if (match(TokenType::RightParen))
                        break; // 尾部逗号
                }
            }

            // 返回类型（接口方法必须有返回类型）
            Expr *returnType = nullptr;
            if (match(TokenType::RightArrow))
            {
                auto res = parseTypeExpr();
                if (!res)
                    return std::unexpected(res.error());
                returnType = *res;
            }
            else
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "interface method must specify return type with `->`",
                    "add `-> ReturnType`",
                    makeSourceLocation(methodNameToken)));
            }

            // 默认实现？（方法签名后直接跟 `{` 表示默认实现）
            BlockStmt *defaultBody = nullptr;
            if (currentToken().type == TokenType::LeftBrace)
            {
                auto bodyRes = parseBlockStmt();
                if (!bodyRes)
                    return std::unexpected(bodyRes.error());
                defaultBody = *bodyRes;
            }
            else if (!match(TokenType::Semicolon))
            {
                return std::unexpected(makeExpectSemicolonError());
            }

            // 构建方法签名
            InterfaceDefStmt::Method method;
            method.name     = methodName;
            method.params   = paramTypes;
            method.retType  = returnType;
            method.location = makeSourceLocation(methodNameToken);
            ifaceDef->methods.push_back(std::move(method));

            // TODO: 存储默认实现体的 AST
            // 当前 InterfaceDefStmt::Method 没有 body 字段
            // 如果需要默认实现，需要扩展 AST
            (void) defaultBody;
        }

        return ifaceDef;
    }

    Result<Stmt *, Error> Parser::parseImpl()
    {
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `impl`

        // impl InterfaceName for StructName { methods... }
        auto interfaceTypeRes = parseTypeExpr();
        if (!interfaceTypeRes)
            return std::unexpected(interfaceTypeRes.error());
        Expr *interfaceType = *interfaceTypeRes;

        if (!match(TokenType::For))
        {
            return std::unexpected(
                makeUnexpectTokenError("ImplStmt", "`for` keyword", currentToken()));
        }

        auto structTypeRes = parseTypeExpr();
        if (!structTypeRes)
            return std::unexpected(structTypeRes.error());
        Expr *structType = *structTypeRes;

        if (!match(TokenType::LeftBrace))
        {
            return std::unexpected(
                makeUnexpectTokenError("ImplStmt", "lbrace `{`", currentToken()));
        }

        const Token &lbToken = prevToken();

        DynArray<FnDefStmt *> methods;

        while (true)
        {
            if (isEOF)
            {
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "unclosed braces in impl block",
                    "insert '}'",
                    makeSourceLocation(lbToken)));
            }
            if (match(TokenType::RightBrace))
            {
                break;
            }

            // impl 方法需要 `func` 关键字
            if (currentToken().type != TokenType::Function)
            {
                return std::unexpected(
                    makeUnexpectTokenError("ImplStmt", "`func` for method", currentToken()));
            }
            consumeToken(); // consume `func`

            if (!currentToken().isIdentifier())
            {
                return std::unexpected(
                    makeUnexpectTokenError("ImplStmt", "method name", currentToken()));
            }

            const Token  &methodNameToken = consumeToken();
            const String &methodName =
                srcManager.GetSub(methodNameToken.index, methodNameToken.length);

            // 参数列表
            if (!match(TokenType::LeftParen))
            {
                return std::unexpected(
                    makeUnexpectTokenError("ImplStmt", "`(` for method params", currentToken()));
            }

            // 复用 FnDefStmt 来解析 impl 方法
            // 但我们不使用 parseFnDefStmt，因为 impl 方法:
            // 1. 不写返回类型（由 interface 约束）
            // 2. 必须有函数体（{ }）

            DynArray<Param *> fnParams;

            while (true)
            {
                if (isEOF)
                {
                    return std::unexpected(Error(
                        ErrorType::SyntaxError,
                        "unclosed parenthese in impl method params",
                        "insert ')'",
                        makeSourceLocation(methodNameToken)));
                }
                if (match(TokenType::RightParen))
                {
                    break;
                }

                if (!currentToken().isIdentifier())
                {
                    return std::unexpected(
                        makeUnexpectTokenError("ImplStmt", "param name", currentToken()));
                }

                const Token  &pToken = consumeToken();
                const String &pName  = srcManager.GetSub(pToken.index, pToken.length);

                Expr *pType = nullptr;
                if (match(TokenType::Colon))
                {
                    auto res = parseTypeExpr();
                    if (!res)
                        return std::unexpected(res.error());
                    pType = *res;
                }

                PosParam *param = arena.Allocate<PosParam>(
                    pName, pType, nullptr, makeSourceLocation(pToken));
                fnParams.push_back(param);

                if (match(TokenType::Comma))
                {
                    if (match(TokenType::RightParen))
                        break; // 尾部逗号
                }
            }

            // impl 方法不写返回类型，必须有函数体
            BlockStmt *methodBody = nullptr;
            if (currentToken().type == TokenType::LeftBrace)
            {
                auto bodyRes = parseBlockStmt();
                if (!bodyRes)
                    return std::unexpected(bodyRes.error());
                methodBody = *bodyRes;
            }
            else
            {
                return std::unexpected(
                    makeUnexpectTokenError("ImplStmt", "`{` for method body", currentToken()));
            }

            FnDefStmt *fnDef = arena.Allocate<FnDefStmt>(
                false, methodName, fnParams, nullptr /* no return type */, methodBody,
                makeSourceLocation(methodNameToken));
            methods.push_back(fnDef);
        }

        ImplStmt *implStmt = arena.Allocate<ImplStmt>(interfaceType, structType, methods, location);
        return implStmt;
    }

    Result<Stmt *, Error> Parser::parseImportStmt()
    {
        SourceLocation location = makeSourceLocation(consumeToken()); // consume `import`

        bool   isFileImport = false;
        String path;

        if (currentToken().type == TokenType::LiteralString)
        {
            // import "path/to/file.fig" — 文件导入
            isFileImport = true;
            const Token &strToken = consumeToken();
            path = srcManager.GetSub(strToken.index, strToken.length);
        }
        else if (currentToken().isIdentifier())
        {
            // import std.io — Module 导入
            isFileImport = false;
            // 收集完整的路径: a.b.c
            while (true)
            {
                const Token &tok = consumeToken();
                path += srcManager.GetSub(tok.index, tok.length);
                if (currentToken().type == TokenType::Dot)
                {
                    path += ".";
                    consumeToken(); // consume `.`
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            return std::unexpected(
                makeUnexpectTokenError("ImportStmt", "module path or file string", currentToken()));
        }

        if (!match(TokenType::Semicolon))
        {
            return std::unexpected(makeExpectSemicolonError());
        }

        ImportStmt *importStmt = arena.Allocate<ImportStmt>(path, isFileImport, location);
        return importStmt;
    }

    Result<Stmt *, Error> Parser::parseStatement()
    {
        StateProtector p(this, {State::Standby});

        bool isPublic = match(TokenType::Public);

        TokenType tt = currentToken().type;

        
        switch (tt)
        {
            case TokenType::LeftBrace: return parseBlockStmt();
            case TokenType::If:        return parseIfStmt();
            case TokenType::While:     return parseWhileStmt();
            case TokenType::For:       return parseForStmt();
            case TokenType::Import:    return parseImportStmt();
            case TokenType::Interface: return parseInterfaceDef(isPublic);
            case TokenType::Implement: return parseImpl();

            case TokenType::Variable: return parseVarDecl(isPublic);
            case TokenType::Const:    return parseConstDecl(isPublic);

            case TokenType::Return:
            {
                return parseReturnStmt();
            }

            case TokenType::Function:
            {
                // 需要 lookahead: `func identifier` → 函数定义
                //                 `func (` 或 `func {` → Lambda 表达式语句
                if (peekToken().isIdentifier())
                    return parseFnDefStmt(isPublic);
                goto expr_stmt;
            }

            case TokenType::Struct:
                return parseStructDef(isPublic);

            case TokenType::Break:
            {
                consumeToken();
                SourceLocation loc = makeSourceLocation(prevToken());
                if (!match(TokenType::Semicolon))
                    return std::unexpected(makeExpectSemicolonError());
                return static_cast<Stmt *>(arena.Allocate<BreakStmt>(loc));
            }

            case TokenType::Continue:
            {
                consumeToken();
                SourceLocation loc = makeSourceLocation(prevToken());
                if (!match(TokenType::Semicolon))
                    return std::unexpected(makeExpectSemicolonError());
                return static_cast<Stmt *>(arena.Allocate<ContinueStmt>(loc));
            }

            case TokenType::EndOfFile:
                return nullptr;

            case TokenType::Semicolon:
                return std::unexpected(Error(
                    ErrorType::SyntaxError,
                    "null statement is not allowed here",
                    "remove `;`",
                    makeSourceLocation(currentToken())));

            default:
                break;
        }

    expr_stmt:
    {
        // 表达式语句 (fallback)
        const auto &expr_result = parseExpression();
        if (!expr_result)
            return std::unexpected(expr_result.error());
        ExprStmt *exprStmt = arena.Allocate<ExprStmt>(*expr_result);
        if (!match(TokenType::Semicolon))
            return std::unexpected(makeExpectSemicolonError());
        return exprStmt;
    }
    }

}; // namespace Fig
