#include "Evaluator/Value/value.hpp"
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{
    RvObject Evaluator::evalUnary(Ast::UnaryExpr un, ContextPtr ctx)
    {
        using Ast::Operator;
        Operator op = un->op;
        Ast::Expression exp = un->exp;
        ObjectPtr value = eval(exp, ctx);

        const auto &tryInvokeOverloadFn = [ctx, op](const ObjectPtr &rhs, const std::function<ObjectPtr()> &rollback) {
            if (rhs->is<StructInstance>())
            {
                // 运算符重载
                const TypeInfo &type = actualType(rhs);
                if (ctx->hasOperatorImplemented(type, op))
                {
                    const auto &fnOpt = ctx->getUnaryOperatorFn(type, op);
                    return (*fnOpt)(rhs);
                }
            }
            return rollback();
        };

        switch (op)
        {
            case Operator::Not: {
                return tryInvokeOverloadFn(value, [value]() { return std::make_shared<Object>(!(*value)); });
            }
            case Operator::Subtract: {
                return tryInvokeOverloadFn(value, [value]() { return std::make_shared<Object>(-(*value)); });
            }
            case Operator::BitNot: {
                return tryInvokeOverloadFn(value, [value]() { return std::make_shared<Object>(bit_not(*value)); });
            }
            default: {
                throw EvaluatorError(u8"UnsupportedOpError",
                                     std::format("Unsupported op '{}' for unary expression", magic_enum::enum_name(op)),
                                     un);
            }
        }
    }
}; // namespace Fig