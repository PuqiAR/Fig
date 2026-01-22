#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{

    RvObject Evaluator::evalTernary(Ast::TernaryExpr te, ContextPtr ctx)
    {
        RvObject condVal = eval(te->condition, ctx);
        if (condVal->getTypeInfo() != ValueType::Bool)
        {
            throw EvaluatorError(
                u8"TypeError",
                std::format("Condition must be boolean, got '{}'", prettyType(condVal).toBasicString()),
                te->condition);
        }
        ValueType::BoolClass cond = condVal->as<ValueType::BoolClass>();
        return (cond ? eval(te->valueT, ctx) : eval(te->valueF, ctx));
    }
};