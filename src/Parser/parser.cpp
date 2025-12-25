#include <Parser/parser.hpp>

namespace Fig
{
    // Operator : pair<LeftBindingPower, RightBindingPower>

    const std::unordered_map<Ast::Operator, std::pair<Parser::Precedence, Parser::Precedence>> Parser::opPrecedence = {
        // 算术
        {Ast::Operator::Add, {10, 11}},
        {Ast::Operator::Subtract, {10, 11}},
        {Ast::Operator::Multiply, {20, 21}},
        {Ast::Operator::Divide, {20, 21}},
        {Ast::Operator::Modulo, {20, 21}},
        {Ast::Operator::Power, {30, 29}},

        // 逻辑
        {Ast::Operator::And, {5, 6}},
        {Ast::Operator::Or, {4, 5}},
        {Ast::Operator::Not, {30, 31}}, // 一元

        // 比较
        {Ast::Operator::Equal, {7, 8}},
        {Ast::Operator::NotEqual, {7, 8}},
        {Ast::Operator::Less, {8, 9}},
        {Ast::Operator::LessEqual, {8, 9}},
        {Ast::Operator::Greater, {8, 9}},
        {Ast::Operator::GreaterEqual, {8, 9}},

        // 位运算
        {Ast::Operator::BitAnd, {6, 7}},
        {Ast::Operator::BitOr, {4, 5}},
        {Ast::Operator::BitXor, {5, 6}},
        {Ast::Operator::BitNot, {30, 31}}, // 一元
        {Ast::Operator::ShiftLeft, {15, 16}},
        {Ast::Operator::ShiftRight, {15, 16}},

        {Ast::Operator::Assign, {2, 1}}, // 右结合

        // 海象运算符
        // {Ast::Operator::Walrus, {2, 1}}, // 右结合

        // // 点运算符
        // {Ast::Operator::Dot, {40, 41}},
    };

    Ast::VarDef Parser::__parseVarDef(bool isPublic)
    {
        // entry: current is keyword `var` or `const`
        bool isConst = (currentToken().getType() == TokenType::Const ? true : false);
        next();
        expect(TokenType::Identifier);
        FString name = currentToken().getValue();
        next();
        FString tiName = ValueType::Any.name;
        bool hasSpecificType = false;
        if (isThis(TokenType::Colon)) // :
        {
            expectPeek(TokenType::Identifier, FString(u8"Type name"));
            next();
            tiName = currentToken().getValue();
            next();
            hasSpecificType = true;
        }
        if (isThis(TokenType::Semicolon))
        {
            next(); // consume `;`, no using expectConsume here cause we don't need to check again
            return makeAst<Ast::VarDefAst>(isPublic, isConst, name, tiName, nullptr);
        }
        if (!isThis(TokenType::Assign) and !isThis(TokenType::Walrus)) expect(TokenType::Assign, u8"assign or walrus");
        if (isThis(TokenType::Walrus))
        {
            if (hasSpecificType) throwAddressableError<SyntaxError>(FString(u8""));
            tiName = Parser::varDefTypeFollowed;
        }
        next();
        Ast::Expression exp = parseExpression(0);
        expectSemicolon();
        return makeAst<Ast::VarDefAst>(isPublic, isConst, name, tiName, exp);
    }

    ObjectPtr Parser::__parseValue()
    {
        FString _val = currentToken().getValue();
        if (currentToken().getType() == TokenType::LiteralNumber)
        {
            if (_val.contains(u8'.') || _val.contains(u8'e'))
            {
                // 非整数
                ValueType::DoubleClass d;
                try
                {
                    d = std::stod(_val.toBasicString());
                }
                catch (...)
                {
                    throwAddressableError<SyntaxError>(FString(u8"Illegal number literal"));
                }
                return std::make_shared<Object>(d);
            }
            else
            {
                // 整数
                ValueType::IntClass i;
                try
                {
                    i = std::stoi(_val.toBasicString());
                }
                catch (...)
                {
                    throwAddressableError<SyntaxError>(FString(u8"Illegal number literal"));
                }
                return std::make_shared<Object>(i);
            }
        }
        else if (currentToken().getType() == TokenType::LiteralString)
        {
            return std::make_shared<Object>(_val);
        }
        else if (currentToken().getType() == TokenType::LiteralBool)
        {
            return std::make_shared<Object>((_val == u8"true" ? true : false));
        }
        else if (currentToken().getType() == TokenType::LiteralNull)
        {
            return Object::getNullInstance();
        }
        else
        {
            throw std::runtime_error(std::string("Internal Error at: ") + std::string(__func__));
        }
    }

    Ast::ValueExpr Parser::__parseValueExpr()
    {
        return makeAst<Ast::ValueExprAst> (__parseValue());
    }
    Ast::FunctionParameters Parser::__parseFunctionParameters()
    {
        // entry: current is Token::LeftParen
        // stop: current is `)` next one
        // *note: must called when parsing function

        next(); // skip `(`

        Ast::FunctionParameters::PosParasType pp;
        Ast::FunctionParameters::DefParasType dp;
        while (true)
        {
            if (isThis(TokenType::RightParen))
            {
                next();
                return Ast::FunctionParameters(pp, dp);
            }
            expect(TokenType::Identifier, FString(u8"Identifier or `)`")); // check current
            FString pname = currentToken().getValue();
            next();                        // skip pname
            if (isThis(TokenType::Assign)) // =
            {
                next();
                dp.push_back({pname, {ValueType::Any.name, parseExpression(0, TokenType::Comma)}});
                if (isThis(TokenType::Comma))
                {
                    next(); // only skip `,` when it's there
                }
            }
            else if (isThis(TokenType::Colon)) // :
            {
                next(); // skip `:`
                expect(TokenType::Identifier, FString(u8"Type name"));
                FString ti(currentToken().getValue());
                next();                        // skip type name
                if (isThis(TokenType::Assign)) // =
                {
                    next(); // skip `=`
                    dp.push_back({pname, {ti, parseExpression(0, TokenType::Comma)}});
                    if (isThis(TokenType::Comma))
                    {
                        next(); // only skip `,` when it's there
                    }
                }
                else
                {
                    pp.push_back({pname, ti});
                    if (isThis(TokenType::Comma))
                    {
                        next(); // only skip `,` when it's there
                    }
                }
            }
            else
            {
                pp.push_back({pname, ValueType::Any.name});
                if (isThis(TokenType::Comma))
                {
                    next(); // only skip `,` when it's there
                }
            }
        }
    }
    Ast::FunctionDef Parser::__parseFunctionDef(bool isPublic)
    {
        FString funcName = currentToken().getValue();
        next();
        expect(TokenType::LeftParen);
        Ast::FunctionParameters params = __parseFunctionParameters();
        FString retTiName = ValueType::Any.name;
        if (isThis(TokenType::RightArrow)) // ->
        {
            next(); // skip `->`
            expect(TokenType::Identifier);
            retTiName = currentToken().getValue();
            next(); // skip return type
        }
        expect(TokenType::LeftBrace);
        Ast::BlockStatement body = __parseBlockStatement();
        return makeAst<Ast::FunctionDefSt>(funcName, params, isPublic, retTiName, body);
    }
    Ast::StructDef Parser::__parseStructDef(bool isPublic)
    {
        // entry: current is struct name
        FString structName = currentToken().getValue();
        next();
        expect(TokenType::LeftBrace, u8"struct body");
        next();
        bool braceClosed = false;

        /*
        public name
        public const name
        public final name

        const name
        final name

        name
        */

        auto __parseStructField = [this](bool isPublic) -> Ast::StructDefField {
            AccessModifier am = AccessModifier::Normal;
            FString fieldName;
            if (isThis(TokenType::Identifier))
            {
                fieldName = currentToken().getValue();
                next();
                am = (isPublic ? AccessModifier::Public : AccessModifier::Normal);
            }
            else if (isThis(TokenType::Const))
            {
                next();
                expect(TokenType::Identifier, u8"field name");
                fieldName = currentToken().getValue();
                am = (isPublic ? AccessModifier::PublicConst : AccessModifier::Const);
            }
            else
            {
                throwAddressableError<SyntaxError>(FString(std::format("expect field name or field attribute")));
            }
            FString tiName = ValueType::Any.name;
            if (isThis(TokenType::Colon))
            {
                next();
                expect(TokenType::Identifier, u8"type name");
                tiName = currentToken().getValue();
                next();
            }
            Ast::Expression initExpr = nullptr;
            if (isThis(TokenType::Assign))
            {
                next();
                if (isEOF()) throwAddressableError<SyntaxError>(FString(u8"expect an expression"));
                initExpr = parseExpression(0);
            }
            expectSemicolon();
            return Ast::StructDefField(am, fieldName, tiName, initExpr);
        };
        std::vector<Ast::Statement> stmts;
        std::vector<Ast::StructDefField> fields;

        while (!isEOF())
        {
            if (isThis(TokenType::RightBrace))
            {
                braceClosed = true;
                next(); // consume `}`
                break;
            }
            if (isThis(TokenType::Identifier))
            {
                fields.push_back(__parseStructField(false));
            }
            else if (isThis(TokenType::Public))
            {
                if (isNext(TokenType::Const))
                {
                    next();
                    fields.push_back(__parseStructField(true));
                }
                else if (isNext(TokenType::Function))
                {
                    next(); // consume `public`
                    next(); // consume `function`
                    stmts.push_back(__parseFunctionDef(true));
                }
                else if (isNext(TokenType::Struct))
                {
                    next(); // consume `public`
                    next(); // consume `struct`
                    stmts.push_back(__parseStructDef(true));
                }
                else if (isNext(TokenType::Identifier))
                {
                    next(); // consume `public`
                    fields.push_back(__parseStructField(true));
                }
                else
                {
                    throwAddressableError<SyntaxError>(FString("Invalid syntax"));
                }
            }
            else if (isThis(TokenType::Function))
            {
                next();
                stmts.push_back(__parseFunctionDef(false));
            }
            else if (isThis(TokenType::Struct))
            {
                next(); // consume `struct`
                stmts.push_back(__parseStructDef(false));
            }
            else if (isThis(TokenType::Const))
            {
                fields.push_back(__parseStructField(false));
            }
            else if (isThis(TokenType::Variable))
            {
                throwAddressableError<SyntaxError>(FString("Variables are not allowed to be defined within a structure."));
            }
            else
            {
                throwAddressableError<SyntaxError>(FString("Invalid syntax"));
            }
        }
        if (!braceClosed)
        {
            throwAddressableError<SyntaxError>(FString("braces are not closed"));
        }
        return makeAst<Ast::StructDefSt>(isPublic, structName, fields, makeAst<Ast::BlockStatementAst>(stmts));
    }
    Ast::Statement Parser::__parseStatement()
    {
        Ast::Statement stmt;
        if (isThis(TokenType::EndOfFile)) { return makeAst<Ast::EofStmt>(); }
        if (isThis(TokenType::Public))
        {
            next(); // consume `public`
            if (isThis(TokenType::Variable) || isThis(TokenType::Const))
            {
                stmt = __parseVarDef(true);
            }
            else if (isThis(TokenType::Function) and isNext(TokenType::Identifier))
            {
                next();
                stmt = __parseFunctionDef(true);
            }
            else if (isThis(TokenType::Struct))
            {
                stmt = __parseStructDef(true);
            }
            else
            {
                throwAddressableError<SyntaxError>(FString(u8"Expected `var`, `const`, `function` or `struct` after `public`"));
            }
        }
        else if (isThis(TokenType::Variable) || isThis(TokenType::Const))
        {
            stmt = __parseVarDef(false);
        }
        else if (isThis(TokenType::Function) and isNext(TokenType::Identifier))
        {
            next();
            stmt = __parseFunctionDef(false);
        }
        else if (isThis(TokenType::Struct))
        {
            expectPeek(TokenType::Identifier, u8"struct name");
            next();
            stmt = __parseStructDef(false);
        }
        else if (isThis(TokenType::If))
        {
            stmt = __parseIf();
        }
        else if (isThis(TokenType::Else))
        {
            throwAddressableError<SyntaxError>(FString(u8"`else` without matching `if`"));
        }
        else if (isThis(TokenType::LeftBrace))
        {
            stmt = __parseBlockStatement();
        }
        else if (isThis(TokenType::While))
        {
            stmt = __parseWhile();
        }
        else if (isThis(TokenType::For))
        {
            stmt = __parseFor();
        }
        else if (isThis(TokenType::Return))
        {
            stmt = __parseReturn();
        }
        else if (isThis(TokenType::Break))
        {
            stmt = __parseBreak();
        }
        else if (isThis(TokenType::Continue))
        {
            stmt = __parseContinue();
        }
        else
        {
            // expression statement
            Ast::Expression exp = parseExpression(0);
            expectSemicolon();
            stmt = makeAst<Ast::ExpressionStmtAst>(exp);
        }
        return stmt;
    }
    Ast::BlockStatement Parser::__parseBlockStatement()
    {
        // entry: current is `{`
        // stop: current is `}` next one
        next(); // consume `{`
        std::vector<Ast::Statement> stmts;
        while (true)
        {
            if (isThis(TokenType::RightBrace))
            {
                next();
                return makeAst<Ast::BlockStatementAst>(stmts);
            }
            stmts.push_back(__parseStatement());
        }
    }
    Ast::If Parser::__parseIf()
    {
        // entry: current is `if`
        next(); // consume `if`
        Ast::Expression condition;
        if (isThis(TokenType::LeftParen))
        {
            next(); // consume `(`
            condition = parseExpression(0, TokenType::RightParen);
            expect(TokenType::RightParen);
            next(); // consume `)`
        }
        else
        {
            condition = parseExpression(0);
        }
        // parenthesis is not required
        expect(TokenType::LeftBrace); // {
        Ast::BlockStatement body = __parseBlockStatement();
        std::vector<Ast::ElseIf> elifs;
        Ast::Else els = nullptr;
        while (isThis(TokenType::Else))
        {
            next(); // consume `else`
            if (isThis(TokenType::If))
            {
                // else if
                next(); // consume `if`
                Ast::Expression elifCondition = parseExpression(0);
                expect(TokenType::LeftBrace); // {
                Ast::BlockStatement elifBody = __parseBlockStatement();
                elifs.push_back(makeAst<Ast::ElseIfSt>(elifCondition, elifBody));
            }
            else
            {
                expect(TokenType::LeftBrace); // {
                Ast::BlockStatement elseBody = __parseBlockStatement();
                els = makeAst<Ast::ElseSt>(elseBody);
                break;
            }
        }
        return makeAst<Ast::IfSt>(condition, body, elifs, els);
    }
    Ast::While Parser::__parseWhile()
    {
        // entry: current is `while`
        next(); // consume `while`
        Ast::Expression condition = parseExpression(0);
        expect(TokenType::LeftBrace); // {
        Ast::BlockStatement body = __parseBlockStatement();
        return makeAst<Ast::WhileSt>(condition, body);
    }
    Ast::Statement Parser::__parseIncrementStatement()
    {
        // allowed：
        // 1. assignment：i = 1, i += 1
        // 2. expression stmt：i++, foo()
        // ❌ not allowed：if/while/for/block stmt

        if (isThis(TokenType::LeftBrace))
        {
            throwAddressableError<SyntaxError>(u8"BlockStatement cannot be used as for loop increment");
        }

        if (isThis(TokenType::If) || isThis(TokenType::While) || isThis(TokenType::For) || isThis(TokenType::Return) || isThis(TokenType::Break) || isThis(TokenType::Continue))
        {
            throwAddressableError<SyntaxError>(u8"Control flow statements cannot be used as for loop increment");
        }

        return __parseStatement();
    }
    Ast::For Parser::__parseFor()
    {
        // entry: current is `for`
        // TODO: support enumeration
        next(); // consume `for`
        bool paren = isThis(TokenType::LeftParen);
        if (paren)
            next(); // consume `(`
        // support 3-part for loop
        // for init; condition; increment {}
        Ast::Statement initStmt = __parseStatement(); // auto check ``
        Ast::Expression condition = parseExpression(0);
        expectSemicolon(); // auto consume `;`

        Ast::Statement incrementStmt = nullptr;
        if (!isThis(paren ? TokenType::RightParen : TokenType::LeftBrace)) // need parse increment?
        {
            auto guard = disableSemicolon();
            incrementStmt = __parseIncrementStatement();
        } // after parse increment, semicolon check state restored
        if (paren)
            expectConsume(TokenType::RightParen);           // consume `)` if has `(`
        expect(TokenType::LeftBrace);                       // {
        Ast::BlockStatement body = __parseBlockStatement(); // auto consume `}`
        return makeAst<Ast::ForSt>(initStmt, condition, incrementStmt, body);
    }
    Ast::Return Parser::__parseReturn()
    {
        // entry: current is `return`
        next(); // consume `return`
        Ast::Expression retValue = parseExpression(0);
        expectSemicolon();
        return makeAst<Ast::ReturnSt>(retValue);
    }
    Ast::Continue Parser::__parseContinue()
    {
        // entry: current is `continue`
        next(); // consume `continue`
        expectSemicolon();
        return makeAst<Ast::ContinueSt>();
    }
    Ast::Break Parser::__parseBreak()
    {
        // entry: current is `break`
        next(); // consume `break`
        expectSemicolon();
        return makeAst<Ast::BreakSt>();
    }

    Ast::VarExpr Parser::__parseVarExpr(FString name)
    {
        return makeAst<Ast::VarExprAst>(name);
    }

    Ast::UnaryExpr Parser::__parsePrefix(Ast::Operator op, Precedence bp)
    {
        return makeAst<Ast::UnaryExprAst>(op, parseExpression(bp));
    }
    Ast::BinaryExpr Parser::__parseInfix(Ast::Expression lhs, Ast::Operator op, Precedence bp)
    {
        return makeAst<Ast::BinaryExprAst>(lhs, op, parseExpression(bp));
    }

    Ast::Expression Parser::__parseCall(Ast::Expression callee)
    {
        next(); // consume '('
        std::vector<Ast::Expression> args;

        if (!isThis(TokenType::RightParen))
        {
            while (true)
            {
                args.push_back(parseExpression(0, TokenType::Comma, TokenType::RightParen));

                if (isThis(TokenType::Comma))
                {
                    next();
                    continue;
                }
                break;
            }
        }

        expect(TokenType::RightParen);
        next(); // consume ')'

        return makeAst<Ast::FunctionCallExpr>(callee, Ast::FunctionArguments(args));
    }

    Ast::ListExpr Parser::__parseListExpr()
    {
        // entry: current is `[`
        next(); // consume `[`
        std::vector<Ast::Expression> val;
        while (!isThis(TokenType::RightBracket))
        {
            val.push_back(parseExpression(0, TokenType::RightBracket, TokenType::Comma));
            if (isThis(TokenType::Comma))
            {
                next(); // consume `,`
            }
        }
        expect(TokenType::RightBracket);
        next(); // consume `]`
        return makeAst<Ast::ListExprAst>(val);
    }

    Ast::MapExpr Parser::__parseMapExpr()
    {
        // entry: current is `{`
        next(); // consume `{`
        std::map<Ast::Expression, Ast::Expression> val;
        while (!isThis(TokenType::RightBrace))
        {
            Ast::Expression key = parseExpression(0, TokenType::Colon);
            expect(TokenType::Colon);
            next(); // consume `:`
            val[key] = parseExpression(0, TokenType::RightBrace, TokenType::Comma);
            if (isThis(TokenType::Comma))
            {
                next(); // consume `,`
            }
        }
        expect(TokenType::RightBrace);
        next(); // consume `}`
        return makeAst<Ast::MapExprAst>(val);
    }

    Ast::InitExpr Parser::__parseInitExpr(FString structName)
    {
        // entry: current is `{`
        next(); // consume `{`
        std::vector<std::pair<FString, Ast::Expression>> args;
        /*
        3 ways of calling constructor
        .1 Person {"Fig", 1, "IDK"};
        .2 Person {name: "Fig", age: 1, sex: "IDK"}; // can be unordered
        .3 Person {name, age, sex};
        */
        uint8_t mode; // 0=undetermined, 1=positional, 2=named, 3=shorthand

        while (!isThis(TokenType::RightBrace))
        {
            if (mode == 0)
            {
                if (isThis(TokenType::Identifier) && isNext(TokenType::Colon))
                {
                    mode = 2;
                }
                else if (isThis(TokenType::Identifier) && (isNext(TokenType::Comma) || isNext(TokenType::RightBrace)))
                {
                    mode = 3;
                }
                else
                {
                    mode = 1;
                }
            }

            if (mode == 1)
            {
                // 1 Person {"Fig", 1, "IDK"};
                Ast::Expression expr = parseExpression(0, TokenType::Comma, TokenType::RightBrace);
                args.push_back({FString(), std::move(expr)});
            }
            else if (mode == 2)
            {
                // 2 Person {name: "Fig", age: 1, sex: "IDK"};
                expect(TokenType::Identifier);
                FString fieldName = currentToken().getValue();
                next(); // consume identifier
                expect(TokenType::Colon);
                next(); // consume colon
                Ast::Expression expr = parseExpression(0, TokenType::Comma, TokenType::RightBrace);
                args.push_back({fieldName, std::move(expr)});
            }
            else if (mode == 3)
            {
                // 3 Person {name, age, sex};
                expect(TokenType::Identifier);
                FString fieldName = currentToken().getValue();
                Ast::Expression expr = makeAst<Ast::VarExprAst>(fieldName);
                args.push_back({fieldName, std::move(expr)});
                next(); // consume identifier
            }

            if (isThis(TokenType::Comma))
            {
                next(); // consume comma
            }
            else if (!isThis(TokenType::RightBrace))
            {
                throwAddressableError<SyntaxError>(FString(
                    std::format("Expect `,` or `}}` in struct initialization expression, got {}",
                                currentToken().toString().toBasicString())
                ));
            }
        }
        expect(TokenType::RightBrace);
        next(); // consume `}`
        return makeAst<Ast::InitExprAst>(structName, args,
                                         (mode == 1 ? Ast::InitExprAst::InitMode::Positional :
                                                      (mode == 2 ? Ast::InitExprAst::InitMode::Named : Ast::InitExprAst::InitMode::Shorthand)));
    }
    Ast::Expression Parser::__parseTupleOrParenExpr()
    {
        next();

        if (currentToken().getType() == TokenType::RightParen)
        {
            next(); // consume ')'
            return makeAst<Ast::TupleExprAst>();
        }

        Ast::Expression firstExpr = parseExpression(0);

        if (currentToken().getType() == TokenType::Comma)
        {
            std::vector<Ast::Expression> elements;
            elements.push_back(firstExpr);

            while (currentToken().getType() == TokenType::Comma)
            {
                next(); // consume ','

                if (currentToken().getType() == TokenType::RightParen)
                    break;

                elements.push_back(parseExpression(0));
            }

            expect(TokenType::RightParen);
            next(); // consume ')'

            return makeAst<Ast::TupleExprAst>(std::move(elements));
        }
        else if (currentToken().getType() == TokenType::RightParen)
        {
            next(); // consume ')'
            return firstExpr;
        }
        else
        {
            throwAddressableError<SyntaxError>(FString(u8"Expect ')' or ',' after expression in parentheses"));
        }
        return nullptr; // to suppress compiler warning
    }

    Ast::FunctionLiteralExpr Parser::__parseFunctionLiteralExpr()
    {
        // entry: current is Token::LeftParen and last is Token::Function
        /*
        Function literal:
            func (params){...}
            or
            func (params) => <expression>
        */
        Ast::FunctionParameters params = __parseFunctionParameters();
        if (isThis(TokenType::DoubleArrow)) // =>
        {
            next();
            Ast::Expression bodyExpr = parseExpression(0);
            return makeAst<Ast::FunctionLiteralExprAst>(params, bodyExpr);
        }
        expect(TokenType::LeftBrace); // `{`
        return makeAst<Ast::FunctionLiteralExprAst>(params, __parseBlockStatement());
    }

    Ast::Expression Parser::parseExpression(Precedence bp, TokenType stop, TokenType stop2)
    {
        Ast::Expression lhs;
        Ast::Operator op;

        Token tok = currentToken();
        if (tok == EOFTok)
            throwAddressableError<SyntaxError>(FString(u8"Unexpected end of expression"));
        if (tok.getType() == stop || tok.getType() == stop2)
        {
            if (lhs == nullptr) throwAddressableError<SyntaxError>(FString(u8"Expected expression"));
            return lhs;
        }
        if (tok.getType() == TokenType::LeftBracket)
        {
            lhs = __parseListExpr(); // auto consume
        }
        else if (tok.getType() == TokenType::LeftParen)
        {
            lhs = __parseTupleOrParenExpr(); // auto consume
        }
        else if (tok.getType() == TokenType::LeftBrace)
        {
            lhs = __parseMapExpr(); // auto consume
        }
        else if (tok.getType() == TokenType::Function)
        {
            next(); // consume `function`
            if (currentToken().getType() == TokenType::Identifier)
            {
                // err
                throwAddressableError<SyntaxError>(FString(u8"Function literal should not have a name"));
            }
            expect(TokenType::LeftParen);
            lhs = __parseFunctionLiteralExpr();
        }
        else if (tok.isLiteral())
        {
            lhs = __parseValueExpr();
            next();
        }
        else if (tok.isIdentifier())
        {
            FString id = tok.getValue();
            next();
            if (currentToken().getType() == TokenType::LeftBrace)
            {
                lhs = __parseInitExpr(id); // a_struct{init...}
            }
            else
            {
                lhs = __parseVarExpr(id);
            }
        }
        else if (isTokenOp(tok) && isOpUnary((op = Ast::TokenToOp.at(tok.getType()))))
        {
            // prefix
            next();
            lhs = __parsePrefix(op, getRightBindingPower(op));
        }
        else
        {
            throwAddressableError<SyntaxError>(FString(u8"Unexpected token in expression"));
        }

        // infix / (postfix) ?
        while (true)
        {
            tok = currentToken();
            if (tok.getType() == TokenType::Semicolon || tok == EOFTok) break;

            /* Postfix */

            if (tok.getType() == TokenType::LeftParen)
            {
                lhs = __parseCall(lhs);
                continue;
            }

            // member access: a.b
            if (tok.getType() == TokenType::Dot)
            {
                next(); // consume '.'
                Token idTok = currentToken();
                if (!idTok.isIdentifier())
                    throwAddressableError<SyntaxError>(FString(u8"Expected identifier after '.'"));

                FString member = idTok.getValue();
                next(); // consume identifier

                lhs = makeAst<Ast::MemberExprAst>(lhs, member);
                continue;
            }
            // index: x[expr]
            if (tok.getType() == TokenType::LeftBracket)
            {
                next(); // consume '['
                auto indexExpr = parseExpression(0, TokenType::RightBracket);
                expect(TokenType::RightBracket);
                next(); // consume ']'

                lhs = makeAst<Ast::IndexExprAst>(lhs, indexExpr);
                continue;
            }

            // ternary
            if (tok.getType() == TokenType::Question)
            {
                next(); // consume ?
                Ast::Expression trueExpr = parseExpression(0, TokenType::Colon);
                expect(TokenType::Colon);
                next(); // consume :
                Ast::Expression falseExpr = parseExpression(0, TokenType::Semicolon, stop2);
                lhs = makeAst<Ast::TernaryExprAst>(lhs, trueExpr, falseExpr);
                continue;
            }

            if (!isTokenOp(tok)) break;

            op = Ast::TokenToOp.at(tok.getType());
            Precedence lbp = getLeftBindingPower(op);
            if (bp >= lbp) break;

            next(); // consume op
            lhs = __parseInfix(lhs, op, getRightBindingPower(op));
        }

        return lhs;
    }

    std::vector<Ast::AstBase> Parser::parseAll()
    {
        output.clear();
        Token tok = currentToken();
        if (tok == EOFTok)
        {
            return output;
        }

        // TODO: Package/Module Import Support
        while (!isEOF())
        {
            pushNode(__parseStatement());
        }
        return output;
    }

} // namespace Fig