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
        switch (op)
        {
            case Operator::Not: {
                return std::make_shared<Object>(!(*value));
            }
            case Operator::Subtract: {
                return std::make_shared<Object>(-(*value));
            }
            case Operator::BitNot: {
                return std::make_shared<Object>(bit_not((*value)));
            }
            default: {
                throw EvaluatorError(u8"UnsupportedOpError",
                                     std::format("Unsupported op '{}' for unary expression", magic_enum::enum_name(op)),
                                     un);
            }
        }
    }
};