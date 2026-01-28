#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{
    RvObject Evaluator::eval(Ast::Expression exp, ContextPtr ctx)
    {
        using Ast::AstType;
        AstType type = exp->getType();
        switch (type)
        {
            case AstType::ValueExpr: {
                auto val = std::static_pointer_cast<Ast::ValueExprAst>(exp);
               
                return val->val;
            }
            case AstType::VarExpr: {
                auto varExpr = std::static_pointer_cast<Ast::VarExprAst>(exp);
               
                return evalVarExpr(varExpr, ctx).get(); // LvObject -> RvObject
            }
            case AstType::BinaryExpr: {
                auto bin = std::static_pointer_cast<Ast::BinaryExprAst>(exp);
               
                return evalBinary(bin, ctx);
            }
            case AstType::UnaryExpr: {
                auto un = std::static_pointer_cast<Ast::UnaryExprAst>(exp);
               
                return evalUnary(un, ctx);
            }
            case AstType::TernaryExpr: {
                auto te = std::static_pointer_cast<Ast::TernaryExprAst>(exp);
               
                return evalTernary(te, ctx);
            }
            case AstType::MemberExpr:
            case AstType::IndexExpr: return evalLv(exp, ctx).get();

            case AstType::FunctionCall: {
                auto fnCall = std::static_pointer_cast<Ast::FunctionCallExpr>(exp);
               
                Ast::Expression callee = fnCall->callee;
                ObjectPtr fnObj = eval(callee, ctx);
                if (fnObj->getTypeInfo() != ValueType::Function)
                {
                    throw EvaluatorError(u8"ObjectNotCallable",
                                         std::format("Object `{}` isn't callable", fnObj->toString().toBasicString()),
                                         callee);
                }
                const Function &fn = fnObj->as<Function>();
                size_t fnId = fn.id;
                // const auto &fnNameOpt = ctx->getFunctionName(fnId);
                // const FString &fnName = (fnNameOpt ? *fnNameOpt :
                // u8"<anonymous>");

                auto fnNameOpt = ctx->getFunctionName(fnId);
                if (!fnNameOpt && fn.closureContext) fnNameOpt = fn.closureContext->getFunctionName(fnId);

                const FString &fnName = (fnNameOpt ? *fnNameOpt : u8"<anonymous> or builtin-type member function");

                return evalFunctionCall(fn, fnCall->arg, fnName, ctx);
            }
            case AstType::FunctionLiteralExpr: {
                auto fnLiteral = std::static_pointer_cast<Ast::FunctionLiteralExprAst>(exp);
               

                Ast::BlockStatement body = nullptr;
                if (fnLiteral->isExprMode())
                {
                    Ast::Expression exprBody = fnLiteral->getExprBody();
                   

                    const Ast::AstAddressInfo &aai = exprBody->getAAI();
                    Ast::Return st = std::make_shared<Ast::ReturnSt>(exprBody);
                    st->setAAI(aai);

                    body = std::make_shared<Ast::BlockStatementAst>();
                    body->stmts.push_back(st); // convert to Ast::Statement
                    body->setAAI(aai);
                }
                else
                {
                    body = fnLiteral->getBlockBody();
                   
                }
                Function fn(fnLiteral->paras, ValueType::Any, body, ctx
                            /*
                                pass the ctx(fnLiteral eval context) as closure context
                            */
                );
                return std::make_shared<Object>(std::move(fn));
            }
            case AstType::InitExpr: {
                auto initExpr = std::static_pointer_cast<Ast::InitExprAst>(exp);
               
                return evalInitExpr(initExpr, ctx);
            }

            case AstType::ListExpr: {
                auto lstExpr = std::static_pointer_cast<Ast::ListExprAst>(exp);
               

                List list;
                for (auto &exp : lstExpr->val) { list.push_back(eval(exp, ctx)); }
                return std::make_shared<Object>(std::move(list));
            }

            case AstType::MapExpr: {
                auto mapExpr = std::static_pointer_cast<Ast::MapExprAst>(exp);
               

                Map map;
                for (auto &[key, value] : mapExpr->val) { map[eval(key, ctx)] = eval(value, ctx); }
                return std::make_shared<Object>(std::move(map));
            }

            default: {
                throw RuntimeError(FString(std::format("err type of expr: {}", magic_enum::enum_name(type))));
            }
        }
        return Object::getNullInstance(); // ignore warning
    }
};