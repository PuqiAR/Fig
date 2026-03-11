/*!
    @file src/Sema/Analyzer.cpp
    @brief 语义分析器实现：实装强类型校验 (Call, Infix, Member, Return, Assign)
*/

#include <Ast/Ast.hpp>
#include <Ast/Expr/MemberExpr.hpp>
#include <Ast/Expr/ObjectInitExpr.hpp>
#include <Ast/Stmt/ImplStmt.hpp>
#include <Ast/Stmt/InterfaceDefStmt.hpp>
#include <Ast/Stmt/StructDefStmt.hpp>
#include <Sema/Analyzer.hpp>

namespace Fig
{
    struct AnalyzerState
    {
        int        loopDepth = 0;
        FnDefStmt *currentFn = nullptr;
    } state;

    struct ScopeGuard
    {
        Environment &env;
        ScopeGuard(Environment &e, bool isFn) : env(e)
        {
            env.Push(isFn);
        }
        ~ScopeGuard()
        {
            env.Pop();
        }
    };

    struct LoopGuard
    {
        int &depth;
        LoopGuard(int &d) : depth(d)
        {
            depth++;
        }
        ~LoopGuard()
        {
            depth--;
        }
    };

    struct FnStateGuard
    {
        FnDefStmt *&current;
        FnDefStmt  *old;
        FnStateGuard(FnDefStmt *&c, FnDefStmt *n) : current(c), old(c)
        {
            current = n;
        }
        ~FnStateGuard()
        {
            current = old;
        }
    };

    Result<void, Error> Analyzer::Analyze(Program *prog)
    {
        ScopeGuard guard(env, false);
        auto       r1 = pass1(prog);
        if (!r1)
            return r1;
        auto r2 = resolveTypes(prog);
        if (!r2)
            return r2;
        auto r3 = checkBodies(prog);
        if (!r3)
            return r3;
        return {};
    }

    Result<void, Error> Analyzer::pass1(Program *prog)
    {
        for (auto *stmt : prog->nodes)
        {
            if (stmt->type == AstType::StructDefStmt)
            {
                auto *s = static_cast<StructDefStmt *>(stmt);
                if (globalTypes.contains(s->name))
                    return std::unexpected(
                        Error(ErrorType::RedeclarationError, "type redeclared", "", s->location));
                auto *t = arena.Allocate<StructType>(s->name);
                typeCtx.allTypes.push_back(t);
                globalTypes[s->name] = t;
            }
            else if (stmt->type == AstType::FnDefStmt)
            {
                auto *f = static_cast<FnDefStmt *>(stmt);
                if (f->name == "main")
                    hasMain = true;
                if (globalSymbols.contains(f->name))
                    return std::unexpected(
                        Error(ErrorType::RedeclarationError, "func redeclared", "", f->location));
                Symbol *sym =
                    arena.Allocate<Symbol>(f->name, Type{}, SymbolLocation::Global, 0, true);
                globalSymbols[f->name]       = sym;
                env.current->locals[f->name] = sym;
                f->resolvedSymbol            = sym;
            }
        }
        return {};
    }

    Result<void, Error> Analyzer::resolveTypes(Program *prog)
    {
        for (auto *stmt : prog->nodes)
        {
            if (stmt->type == AstType::StructDefStmt)
            {
                auto *s  = static_cast<StructDefStmt *>(stmt);
                auto *st = static_cast<StructType *>(globalTypes[s->name]);
                for (auto &f : s->fields)
                {
                    auto res = resolveTypeExpr(f.type);
                    if (!res)
                        return std::unexpected(res.error());
                    st->AddField(f.name, *res, f.isPublic);
                }
            }
            else if (stmt->type == AstType::FnDefStmt)
            {
                auto *f   = static_cast<FnDefStmt *>(stmt);
                auto  res = resolveTypeExpr(f->returnTypeSpecifier);
                if (!res)
                    return std::unexpected(res.error());
                f->resolvedReturnType = *res;

                DynArray<Type> paramTypes;
                for (auto *p : f->params)
                {
                    auto pres = resolveTypeExpr(p->typeSpecifier);
                    if (!pres)
                        return std::unexpected(pres.error());
                    p->resolvedType = *pres;
                    paramTypes.push_back(*pres);
                }

                f->resolvedSymbol->type = typeCtx.CreateFuncType(std::move(paramTypes), *res);
            }
        }
        return {};
    }

    Result<void, Error> Analyzer::checkBodies(Program *prog)
    {
        for (auto *stmt : prog->nodes)
        {
            auto r = analyzeStmt(stmt);
            if (!r)
                return r;
        }
        return {};
    }

    Result<void, Error> Analyzer::analyzeStmt(Stmt *stmt)
    {
        if (!stmt)
            return {};
        switch (stmt->type)
        {
            case AstType::BlockStmt: {
                auto      *b = static_cast<BlockStmt *>(stmt);
                ScopeGuard guard(env, false);
                for (auto *s : b->nodes)
                {
                    if (auto r = analyzeStmt(s); !r)
                        return r;
                }
                break;
            }
            case AstType::VarDecl: {
                auto *v     = static_cast<VarDecl *>(stmt);
                Type  initT = typeCtx.GetBasic(TypeTag::Any);
                if (v->initExpr)
                {
                    auto res = analyzeExpr(v->initExpr);
                    if (!res)
                        return std::unexpected(res.error());
                    initT = *res;
                }
                Type declT = v->typeSpecifier ? *resolveTypeExpr(v->typeSpecifier) : initT;

                // 🔥 强类型校验：赋值拦截
                if (v->initExpr && !initT.isAssignableTo(declT))
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "cannot assign '" + initT.toString() + "' to type '" + declT.toString()
                            + "'",
                        "",
                        v->location));
                }

                if (env.current->locals.contains(v->name))
                    return std::unexpected(
                        Error(ErrorType::RedeclarationError, "var redeclared", "", v->location));
                SymbolLocation loc =
                    env.current->parent ? SymbolLocation::Local : SymbolLocation::Global;
                int idx = (loc == SymbolLocation::Local) ? env.current->nextLocalId++ : 0;
                env.current->locals[v->name] =
                    arena.Allocate<Symbol>(v->name, declT, loc, idx, false);
                v->localId = idx;
                break;
            }

            case AstType::FnDefStmt: {
                auto *f = static_cast<FnDefStmt *>(stmt);

                // 3.10: 局部闭包延迟类型推导

                if (!f->resolvedSymbol) // 闭包？
                {
                    SymbolLocation loc =
                        env.current->parent ? SymbolLocation::Local : SymbolLocation::Global;
                    int idx = (loc == SymbolLocation::Local) ? env.current->nextLocalId++ : 0;

                    Symbol *sym       = arena.Allocate<Symbol>(f->name, Type{}, loc, idx, true);
                    f->resolvedSymbol = sym;
                    env.current->locals[f->name] = sym;

                    auto res = resolveTypeExpr(f->returnTypeSpecifier);
                    if (!res)
                        return std::unexpected(res.error());
                    f->resolvedReturnType = *res;

                    DynArray<Type> paramTypes;
                    for (auto *p : f->params)
                    {
                        auto pres = resolveTypeExpr(p->typeSpecifier);
                        if (!pres)
                            return std::unexpected(pres.error());
                        p->resolvedType = *pres;
                        paramTypes.push_back(*pres);
                    }
                    f->resolvedSymbol->type = typeCtx.CreateFuncType(std::move(paramTypes), *res);
                }

                FnStateGuard fnGuard(state.currentFn, f);
                ScopeGuard   scopeGuard(env, true);
                for (auto *p : f->params)
                {
                    env.current->locals[p->name] = arena.Allocate<Symbol>(p->name,
                        p->resolvedType,
                        SymbolLocation::Local,
                        env.current->nextLocalId++,
                        false);
                }
                if (auto r = analyzeStmt(f->body); !r)
                    return r;

                for (const auto &upval : env.current->upvalues)
                {
                    f->upvalues.push_back({static_cast<std::uint8_t>(upval.index), upval.isLocal});
                }

                break;
            }

            case AstType::IfStmt: {
                auto *i = static_cast<IfStmt *>(stmt);

                if (auto c = analyzeExpr(i->cond); !c)
                    return std::unexpected(c.error());
                else if (!c->isAssignableTo(typeCtx.GetBasic(TypeTag::Bool)))
                {
                    return std::unexpected(Error(
                        ErrorType::TypeError, "condition must be Bool", "", i->cond->location));
                }
                if (auto b = analyzeStmt(i->consequent); !b)
                    return b;

                for (auto *elif : i->elifs)
                {
                    if (auto c = analyzeExpr(elif->cond); !c)
                        return std::unexpected(c.error());
                    else if (!c->isAssignableTo(typeCtx.GetBasic(TypeTag::Bool)))
                    {
                        return std::unexpected(Error(ErrorType::TypeError,
                            "condition must be Bool",
                            "",
                            elif->cond->location));
                    }
                    if (auto b = analyzeStmt(elif->consequent); !b)
                        return b;
                }

                if (i->alternate)
                {
                    if (auto a = analyzeStmt(i->alternate); !a)
                        return a;
                }
                break;
            }
            case AstType::WhileStmt: {
                bool  isWhile = stmt->type == AstType::WhileStmt;
                Expr *cond    = isWhile ? static_cast<WhileStmt *>(stmt)->cond :
                                          static_cast<IfStmt *>(stmt)->cond;
                Stmt *body    = isWhile ? static_cast<WhileStmt *>(stmt)->body :
                                          static_cast<IfStmt *>(stmt)->consequent;

                if (auto c = analyzeExpr(cond); !c)
                    return std::unexpected(c.error());
                else if (!c->isAssignableTo(typeCtx.GetBasic(TypeTag::Bool)))
                {
                    return std::unexpected(
                        Error(ErrorType::TypeError, "condition must be Bool", "", cond->location));
                }

                if (isWhile)
                {
                    LoopGuard loopGuard(state.loopDepth);
                    if (auto b = analyzeStmt(body); !b)
                        return b;
                }
                else
                {
                    if (auto b = analyzeStmt(body); !b)
                        return b;
                    auto *i = static_cast<IfStmt *>(stmt);
                    if (i->alternate)
                    {
                        if (auto a = analyzeStmt(i->alternate); !a)
                            return a;
                    }
                }
                break;
            }
            case AstType::BreakStmt:
            case AstType::ContinueStmt:
                if (state.loopDepth <= 0)
                    return std::unexpected(
                        Error(ErrorType::SyntaxError, "outside loop", "", stmt->location));
                break;
            case AstType::ReturnStmt: {
                auto *rs   = static_cast<ReturnStmt *>(stmt);
                Type  retT = typeCtx.GetBasic(TypeTag::Null);
                if (rs->value)
                {
                    auto res = analyzeExpr(rs->value);
                    if (!res)
                        return std::unexpected(res.error());
                    retT = *res;
                }
                // 🔥 强类型校验：返回值拦截
                if (state.currentFn && !retT.isAssignableTo(state.currentFn->resolvedReturnType))
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "cannot return '" + retT.toString() + "' from function expecting '"
                            + state.currentFn->resolvedReturnType.toString() + "'",
                        "",
                        rs->location));
                }
                break;
            }
            case AstType::ExprStmt: {
                auto res = analyzeExpr(static_cast<ExprStmt *>(stmt)->expr);
                if (!res)
                    return std::unexpected(res.error());
                break;
            }
            default: break;
        }
        return {};
    }

    Result<Type, Error> Analyzer::analyzeExpr(Expr *expr)
    {
        if (!expr)
            return typeCtx.GetBasic(TypeTag::Null);
        switch (expr->type)
        {
            case AstType::LiteralExpr: {
                auto t = static_cast<LiteralExpr *>(expr)->literal.type;
                if (t == TokenType::LiteralNumber)
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Int);
                if (t == TokenType::LiteralString)
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::String);
                if (t == TokenType::LiteralTrue || t == TokenType::LiteralFalse)
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Bool);
                return expr->resolvedType = typeCtx.GetBasic(TypeTag::Null);
            }
            case AstType::IdentiExpr: {
                auto *i   = static_cast<IdentiExpr *>(expr);
                auto  res = resolveSymbolInternal(i->name, i->location, env.current);
                if (!res)
                    return std::unexpected(res.error());
                i->resolvedSymbol         = *res;
                return expr->resolvedType = (*res)->type;
            }
            case AstType::MemberExpr: {
                auto *m         = static_cast<MemberExpr *>(expr);
                auto  targetRes = analyzeExpr(m->target);
                if (!targetRes)
                    return targetRes;

                Type targetType = *targetRes;
                if (targetType.is(TypeTag::Any))
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Any);
                if (!targetType.is(TypeTag::Struct))
                    return std::unexpected(Error(
                        ErrorType::TypeError, "member access requires struct", "", m->location));

                auto *st = static_cast<StructType *>(targetType.base);
                if (!st->fieldMap.contains(m->name))
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "struct '" + st->name + "' has no field named '" + m->name + "'",
                        "",
                        m->location));
                }
                // 字段类型
                return expr->resolvedType = st->fields[st->fieldMap[m->name]].type;
            }
            case AstType::ObjectInitExpr: {
                auto *o   = static_cast<ObjectInitExpr *>(expr);
                auto  res = resolveTypeExpr(o->typeExpr);
                if (!res)
                    return std::unexpected(res.error());
                if (!res->base || res->base->tag != TypeTag::Struct)
                    return std::unexpected(
                        Error(ErrorType::TypeError, "requires struct", "", o->location));
                auto *st = static_cast<StructType *>(res->base);
                for (auto &arg : o->args)
                {
                    if (!arg.name.empty() && !st->fieldMap.contains(arg.name))
                        return std::unexpected(
                            Error(ErrorType::TypeError, "unknown field", "", arg.value->location));
                    auto r = analyzeExpr(arg.value);
                    if (!r)
                        return std::unexpected(r.error());
                    // 字段赋值类型检查
                    if (!arg.name.empty()
                        && !r->isAssignableTo(st->fields[st->fieldMap[arg.name]].type))
                    {
                        return std::unexpected(Error(
                            ErrorType::TypeError, "field type mismatch", "", arg.value->location));
                    }
                }
                return expr->resolvedType = *res;
            }
            case AstType::InfixExpr: {
                auto *in   = static_cast<InfixExpr *>(expr);
                auto  lRes = analyzeExpr(in->left);
                if (!lRes)
                    return lRes;
                auto rRes = analyzeExpr(in->right);
                if (!rRes)
                    return rRes;
                Type l = *lRes;
                Type r = *rRes;

                if (in->op == BinaryOperator::Assign)
                {
                    if (!r.isAssignableTo(l))
                        return std::unexpected(Error(ErrorType::TypeError,
                            "cannot assign '" + r.toString() + "' to '" + l.toString() + "'",
                            "",
                            in->location));
                    return expr->resolvedType = l;
                }

                if (in->op == BinaryOperator::Equal || in->op == BinaryOperator::NotEqual
                    || in->op == BinaryOperator::Greater || in->op == BinaryOperator::Less
                    || in->op == BinaryOperator::GreaterEqual
                    || in->op == BinaryOperator::LessEqual)
                {
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Bool);
                }

                if (l.is(TypeTag::Any) || r.is(TypeTag::Any))
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Any);

                // 算术操作强检查
                if (in->op == BinaryOperator::Add && l.is(TypeTag::String) && r.is(TypeTag::String))
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::String);
                if (l.is(TypeTag::Int) && r.is(TypeTag::Int))
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Int);
                if ((l.is(TypeTag::Int) || l.is(TypeTag::Double))
                    && (r.is(TypeTag::Int) || r.is(TypeTag::Double)))
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Double);

                return std::unexpected(Error(
                    ErrorType::TypeError, "invalid types for binary operator", "", in->location));
            }
            case AstType::CallExpr: {
                auto *c         = static_cast<CallExpr *>(expr);
                auto  calleeRes = analyzeExpr(c->callee);
                if (!calleeRes)
                    return calleeRes;
                Type calleeType = *calleeRes;

                DynArray<Type> argTypes;
                for (auto *a : c->args.args)
                {
                    auto ar = analyzeExpr(a);
                    if (!ar)
                        return std::unexpected(ar.error());
                    argTypes.push_back(*ar);
                }

                if (calleeType.is(TypeTag::Any))
                    return expr->resolvedType = typeCtx.GetBasic(TypeTag::Any);

                // 函数签名校验
                if (!calleeType.is(TypeTag::Function))
                    return std::unexpected(
                        Error(ErrorType::TypeError, "callee is not a function", "", c->location));

                auto *ft = static_cast<FuncType *>(calleeType.base);
                if (ft->paramTypes.size() != argTypes.size())
                {
                    return std::unexpected(Error(ErrorType::TypeError,
                        "expected " + std::to_string(ft->paramTypes.size()) + " arguments, got "
                            + std::to_string(argTypes.size()),
                        "",
                        c->location));
                }
                for (size_t i = 0; i < argTypes.size(); ++i)
                {
                    if (!argTypes[i].isAssignableTo(ft->paramTypes[i]))
                    {
                        return std::unexpected(Error(ErrorType::TypeError,
                            "argument " + std::to_string(i + 1) + " expects '"
                                + ft->paramTypes[i].toString() + "', got '" + argTypes[i].toString()
                                + "'",
                            "",
                            c->args.args[i]->location));
                    }
                }
                return expr->resolvedType = ft->retType;
            }
            default: break;
        }
        return expr->resolvedType = typeCtx.GetBasic(TypeTag::Any);
    }

    Result<Symbol *, Error> Analyzer::resolveSymbolInternal(
        const String &name, const SourceLocation &loc, Scope *s)
    {
        Scope *curr = s;
        while (curr)
        {
            if (curr->locals.contains(name))
                return curr->locals[name];
            if (curr->isFunctionBoundary)
                break;
            curr = curr->parent;
        }
        if (curr && curr->parent)
        {
            auto res = resolveSymbolInternal(name, loc, curr->parent);
            if (!res)
                return res;
            Symbol *outer = *res;
            if (outer->location == SymbolLocation::Global)
                return outer;
            int idx = addUpvalue(curr, outer, outer->location == SymbolLocation::Local);
            return arena.Allocate<Symbol>(
                name, outer->type, SymbolLocation::Upvalue, idx, outer->isConst);
        }
        if (globalSymbols.contains(name))
            return globalSymbols[name];
        return std::unexpected(
            Error(ErrorType::UseUndeclaredIdentifier, "symbol not found", "", loc));
    }

    int Analyzer::addUpvalue(Scope *s, Symbol *t, bool isL)
    {
        for (size_t i = 0; i < s->upvalues.size(); ++i)
            if (s->upvalues[i].target == t)
                return (int) i;
        int idx = (int) s->upvalues.size();
        s->upvalues.push_back({t, idx, isL});
        return idx;
    }

    Result<Type, Error> Analyzer::resolveTypeExpr(TypeExpr *texpr)
    {
        if (!texpr)
            return typeCtx.GetBasic(TypeTag::Any);
        if (texpr->type == AstType::NamedTypeExpr)
        {
            auto *n = static_cast<NamedTypeExpr *>(texpr);
            if (n->path.empty())
                return typeCtx.GetBasic(TypeTag::Any);
            String &root = n->path[0];
            if (root == "Any")
                return typeCtx.GetBasic(TypeTag::Any);
            if (root == "Int")
                return typeCtx.GetBasic(TypeTag::Int);
            if (root == "Double")
                return typeCtx.GetBasic(TypeTag::Double);
            if (root == "String")
                return typeCtx.GetBasic(TypeTag::String);
            if (root == "Bool")
                return typeCtx.GetBasic(TypeTag::Bool);
            if (root == "Null")
                return typeCtx.GetBasic(TypeTag::Null);

            if (globalTypes.contains(root))
                return Type{globalTypes[root], false};

            return std::unexpected(
                Error(ErrorType::UseUndeclaredIdentifier, "unknown type", "", texpr->location));
        }
        if (texpr->type == AstType::NullableTypeExpr)
        {
            auto res = resolveTypeExpr(static_cast<NullableTypeExpr *>(texpr)->inner);
            if (res)
                res->isNullable = true;
            return res;
        }
        return typeCtx.GetBasic(TypeTag::Any);
    }
} // namespace Fig