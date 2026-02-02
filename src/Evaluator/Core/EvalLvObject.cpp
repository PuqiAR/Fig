#include "Evaluator/Value/value.hpp"
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{
    LvObject Evaluator::evalVarExpr(Ast::VarExpr var, ContextPtr ctx)
    {
        const FString &name = var->name;
        if (!ctx->contains(name)) { throw EvaluatorError(u8"UndeclaredIdentifierError", name, var); }
        return LvObject(ctx->get(name), ctx);
    }
    LvObject Evaluator::evalMemberExpr(Ast::MemberExpr me, ContextPtr ctx)
    {
        // LvObject base = evalLv(me->base, ctx);
        RvObject baseVal = eval(me->base, ctx);
        const FString &member = me->member;
        if (baseVal->getTypeInfo() == ValueType::Module)
        {
            const Module &mod = baseVal->as<Module>();
            if (mod.ctx->contains(member) && mod.ctx->isVariablePublic(member))
            {
                return LvObject(mod.ctx->get(member), ctx);
            }
            else
            {
                throw EvaluatorError(u8"VariableNotFoundError",
                                     std::format("`{}` has not variable '{}', check if it is public",
                                                 baseVal->toString().toBasicString(),
                                                 member.toBasicString()),
                                     me->base);
            }
        }
        if (baseVal->hasMemberFunction(member))
        {
            return LvObject(std::make_shared<VariableSlot>(
                                member,
                                std::make_shared<Object>(Function(
                                    member,
                                    [baseVal, member](ObjectPtr self, std::vector<ObjectPtr> args) -> ObjectPtr {
                                        if (self) { return baseVal->getMemberFunction(member)(self, args); }
                                        return baseVal->getMemberFunction(member)(baseVal, args);
                                    },
                                    baseVal->getMemberFunctionParaCount(member))),
                                ValueType::Function,
                                AccessModifier::PublicConst),
                            ctx); // fake l-value
        }

        if (ctx->hasMethodImplemented(baseVal->getTypeInfo(), member))
        {
            // builtin type implementation!
            // e.g. impl xxx for Int

            auto &fn = ctx->getImplementedMethod(baseVal->getTypeInfo(), member);
            Function boundFn(member,
                             fn.paras,
                             fn.retType,
                             fn.body,
                             ctx // current context
            );
            return LvObject(
                std::make_shared<VariableSlot>(
                    member, std::make_shared<Object>(boundFn), ValueType::Function, AccessModifier::PublicConst),
                ctx);
        }

        if (baseVal->getTypeInfo() != ValueType::StructInstance) // and not member function found
        {
            throw EvaluatorError(
                u8"NoAttributeError",
                std::format("`{}` has not attribute '{}'", baseVal->toString().toBasicString(), member.toBasicString()),
                me->base);
        }
        const StructInstance &si = baseVal->as<StructInstance>();
        if (ctx->hasMethodImplemented(si.parentType, member))
        {
            auto &fn = ctx->getImplementedMethod(si.parentType, member);
            Function boundFn(member,
                             fn.paras,
                             fn.retType,
                             fn.body,
                             si.localContext // create a new function and set closure context
                                             // to struct instance context
            );
            return LvObject(
                std::make_shared<VariableSlot>(
                    member, std::make_shared<Object>(boundFn), ValueType::Function, AccessModifier::PublicConst),
                ctx);
        }
        else if (si.localContext->containsInThisScope(member) && si.localContext->isVariablePublic(member))
        {
            return LvObject(si.localContext->get(member), ctx);
        }
        else if (ctx->hasDefaultImplementedMethod(si.parentType, member))
        {
            const auto &ifm = ctx->getDefaultImplementedMethod(si.parentType, member);
            Function fn(member, ifm.paras, actualType(eval(ifm.returnType, ctx)), ifm.defaultBody, ctx);

            return LvObject(std::make_shared<VariableSlot>(
                                member, std::make_shared<Object>(fn), ValueType::Function, AccessModifier::PublicConst),
                            ctx);
        }
        else
        {
            throw EvaluatorError(u8"NoAttributeError",
                                 std::format("`{}` has not attribute '{}' and no interfaces have been implemented it",
                                             baseVal->toString().toBasicString(),
                                             member.toBasicString()),
                                 me->base);
        }
    }
    LvObject Evaluator::evalIndexExpr(Ast::IndexExpr ie, ContextPtr ctx)
    {
        RvObject base = eval(ie->base, ctx);
        RvObject index = eval(ie->index, ctx);

        const TypeInfo &type = base.get()->getTypeInfo();

        if (type == ValueType::List)
        {
            if (index->getTypeInfo() != ValueType::Int)
            {
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Type `List` indices must be `Int`, got '{}'", prettyType(index).toBasicString()),
                    ie->index);
            }
            List &list = base.get()->as<List>();
            ValueType::IntClass indexVal = index->as<ValueType::IntClass>();
            if (indexVal >= list.size())
            {
                throw EvaluatorError(
                    u8"IndexOutOfRangeError",
                    std::format("Index {} out of list `{}` range", indexVal, base->toString().toBasicString()),
                    ie->index);
            }
            return LvObject(base, indexVal, LvObject::Kind::ListElement, ctx);
        }
        else if (type == ValueType::Map) { return LvObject(base, index, LvObject::Kind::MapElement, ctx); }
        else if (type == ValueType::String)
        {
            if (index->getTypeInfo() != ValueType::Int)
            {
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Type `String` indices must be `Int`, got '{}'", prettyType(index).toBasicString()),
                    ie->index);
            }
            FString &string = base->as<ValueType::StringClass>();
            ValueType::IntClass indexVal = index->as<ValueType::IntClass>();
            if (indexVal >= string.length())
            {
                throw EvaluatorError(
                    u8"IndexOutOfRangeError",
                    std::format("Index {} out of string `{}` range", indexVal, base->toString().toBasicString()),
                    ie->index);
            }
            return LvObject(base, indexVal, LvObject::Kind::StringElement, ctx);
        }
        else
        {
            throw EvaluatorError(
                u8"NoSubscriptableError",
                std::format("`{}` object is not subscriptable", base->getTypeInfo().toString().toBasicString()),
                ie->base);
        }
    }
    LvObject Evaluator::evalLv(Ast::Expression exp, ContextPtr ctx)
    {
        using Ast::Operator;
        using Ast::AstType;
        switch (exp->getType())
        {
            case AstType::VarExpr: {
                Ast::VarExpr var = std::static_pointer_cast<Ast::VarExprAst>(exp);

                return evalVarExpr(var, ctx);
            }
            case AstType::MemberExpr: {
                Ast::MemberExpr me = std::static_pointer_cast<Ast::MemberExprAst>(exp);

                return evalMemberExpr(me, ctx);
            }
            case AstType::IndexExpr: {
                Ast::IndexExpr ie = std::static_pointer_cast<Ast::IndexExprAst>(exp);

                return evalIndexExpr(ie, ctx);
            }
            default: {
                throw EvaluatorError(
                    u8"TypeError",
                    std::format("Expression '{}' doesn't refer to a lvalue", exp->typeName().toBasicString()),
                    exp);
            }
        }
    }
}; // namespace Fig