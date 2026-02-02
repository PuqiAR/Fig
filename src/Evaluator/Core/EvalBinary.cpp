#include "Ast/Expressions/BinaryExpr.hpp"
#include <Evaluator/Value/value.hpp>
#include <Evaluator/Value/LvObject.hpp>
#include <Evaluator/Value/IntPool.hpp>
#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

#include <memory>
#include <functional>

namespace Fig
{
    RvObject Evaluator::evalBinary(Ast::BinaryExpr bin, ContextPtr ctx)
    {
        using Ast::Operator;
        Operator op = bin->op;
        Ast::Expression lexp = bin->lexp, rexp = bin->rexp;

        const auto &tryInvokeOverloadFn =
            [ctx, op](const ObjectPtr &lhs, const ObjectPtr &rhs, const std::function<ObjectPtr()> &rollback) {
                if (lhs->is<StructInstance>() && lhs->getTypeInfo() == rhs->getTypeInfo())
                {
                    // 运算符重载
                    const TypeInfo &type = actualType(lhs);
                    if (ctx->hasOperatorImplemented(type, op))
                    {
                        const auto &fnOpt = ctx->getBinaryOperatorFn(type, op);
                        return (*fnOpt)(lhs, rhs);
                    }
                }
                return rollback();
            };

        switch (op)
        {
            case Operator::Add: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() + rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(*lhs + *rhs);
                });
            }
            case Operator::Subtract: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() - rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(*lhs - *rhs);
                });
            }
            case Operator::Multiply: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() * rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(*lhs * *rhs);
                });
            }
            case Operator::Divide: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs / *rhs); });
            }
            case Operator::Modulo: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass lv = lhs->as<ValueType::IntClass>();
                        ValueType::IntClass rv = rhs->as<ValueType::IntClass>();
                        if (rv == 0) { throw ValueError(FString(std::format("Modulo by zero: {} % {}", lv, rv))); }
                        ValueType::IntClass result = lv / rv;
                        ValueType::IntClass r = lv % rv;
                        if (r != 0 && ((lv < 0) != (rv < 0))) { result -= 1; }
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(*lhs % *rhs);
                });
            }

            case Operator::Is: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs, ctx, bin]() {
                    const TypeInfo &lhsType = lhs->getTypeInfo();
                    const TypeInfo &rhsType = rhs->getTypeInfo();

                    if (lhs->is<StructInstance>() && rhs->is<StructType>())
                    {
                        const StructInstance &si = lhs->as<StructInstance>();
                        const StructType &st = rhs->as<StructType>();
                        return std::make_shared<Object>(si.parentType == st.type);
                    }
                    if (lhs->is<StructInstance>() && rhs->is<InterfaceType>())
                    {
                        const StructInstance &si = lhs->as<StructInstance>();
                        const InterfaceType &it = rhs->as<InterfaceType>();
                        return std::make_shared<Object>(implements(si.parentType, it.type, ctx));
                    }

                    if (ValueType::isTypeBuiltin(lhsType) && rhsType == ValueType::StructType)
                    {
                        const StructType &st = rhs->as<StructType>();
                        const TypeInfo &type = st.type;
                        /*
                            如果是内置类型(e.g. Int, String)
                            那么 eval出来String这个字，出来的是StructType
                            而出来的StructType.type就不会是一个独立的TypeInfo,而是内置的ValueType::String
                            依次我们可以判断内置类型

                            e.g:
                                "123" is String
                                  L   OP    R

                            其中 L 类型为 String
                            而 R 类型为 StructType (builtins.hpp) 中注册
                            拿到 R 的 StructType, 其中的 type 为 String
                        */
                        if (lhs->getTypeInfo() == type) { return Object::getTrueInstance(); }
                        return Object::getFalseInstance();
                    }

                    throw EvaluatorError(u8"TypeError",
                                         std::format("Unsupported operator `is` for '{}' && '{}'",
                                                     prettyType(lhs).toBasicString(),
                                                     prettyType(rhs).toBasicString()),
                                         bin->lexp);
                });
            }

            case Operator::BitAnd: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() & rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(bit_and(*lhs, *rhs));
                });
            }
            case Operator::BitOr: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() | rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(bit_or(*lhs, *rhs));
                });
            }
            case Operator::BitXor: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() ^ rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(bit_xor(*lhs, *rhs));
                });
            }
            case Operator::ShiftLeft: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() << rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(shift_left(*lhs, *rhs));
                });
            }
            case Operator::ShiftRight: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);

                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() {
                    if (lhs->is<ValueType::IntClass>() && rhs->is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs->as<ValueType::IntClass>() >> rhs->as<ValueType::IntClass>();
                        return IntPool::getInstance().createInt(result);
                    }
                    return std::make_shared<Object>(shift_right(*lhs, *rhs));
                });
            }

            case Operator::Assign: {
                LvObject lv = evalLv(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                lv.set(rhs);
                return rhs;
            }

            case Operator::And: {
                ObjectPtr lhs = eval(lexp, ctx);
                if (lhs->is<bool>() && !isBoolObjectTruthy(lhs)) { return Object::getFalseInstance(); }
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs && *rhs); });
            }
            case Operator::Or: {
                ObjectPtr lhs = eval(lexp, ctx);
                if (lhs->is<bool>() && isBoolObjectTruthy(lhs)) { return Object::getTrueInstance(); }
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs || *rhs); });
            }
            case Operator::Equal: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs == *rhs); });
            }
            case Operator::NotEqual: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs != *rhs); });
            }
            case Operator::Less: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs < *rhs); });
            }
            case Operator::LessEqual: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs <= *rhs); });
            }
            case Operator::Greater: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs > *rhs); });
            }
            case Operator::GreaterEqual: {
                ObjectPtr lhs = eval(lexp, ctx);
                ObjectPtr rhs = eval(rexp, ctx);
                return tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs >= *rhs); });
            }
            case Operator::PlusAssign: {
                LvObject lv = evalLv(lexp, ctx);
                const ObjectPtr &lhs = lv.get();
                ObjectPtr rhs = eval(rexp, ctx);
                const ObjectPtr &result =
                    tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs + *rhs); });
                lv.set(result);
                return rhs;
            }
            case Operator::MinusAssign: {
                LvObject lv = evalLv(lexp, ctx);
                const ObjectPtr &lhs = lv.get();
                ObjectPtr rhs = eval(rexp, ctx);
                const ObjectPtr &result =
                    tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs - *rhs); });
                lv.set(result);
                return rhs;
            }
            case Operator::AsteriskAssign: {
                LvObject lv = evalLv(lexp, ctx);
                const ObjectPtr &lhs = lv.get();
                ObjectPtr rhs = eval(rexp, ctx);
                const ObjectPtr &result =
                    tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs * *rhs); });
                lv.set(result);
                return rhs;
            }
            case Operator::SlashAssign: {
                LvObject lv = evalLv(lexp, ctx);
                const ObjectPtr &lhs = lv.get();
                ObjectPtr rhs = eval(rexp, ctx);
                const ObjectPtr &result =
                    tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs / *rhs); });
                lv.set(result);
                return rhs;
            }
            case Operator::PercentAssign: {
                LvObject lv = evalLv(lexp, ctx);
                const ObjectPtr &lhs = lv.get();
                ObjectPtr rhs = eval(rexp, ctx);
                const ObjectPtr &result =
                    tryInvokeOverloadFn(lhs, rhs, [lhs, rhs]() { return std::make_shared<Object>(*lhs % *rhs); });
                lv.set(result);
                return rhs;
            }
            default:
                throw EvaluatorError(u8"UnsupportedOp",
                                     std::format("Unsupport operator '{}' for binary", magic_enum::enum_name(op)),
                                     bin);
        }
    }
}; // namespace Fig