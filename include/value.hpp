#pragma once
#include <Value/BaseValue.hpp>
#include <Value/valueError.hpp>
#include <Value/function.hpp>
#include <Value/structType.hpp>
#include <Value/structInstance.hpp>

#include <Value/Type.hpp>

#include <variant>
#include <cmath>
#include <string>
#include <format>

namespace Fig
{
    inline bool isDoubleInteger(ValueType::DoubleClass d)
    {
        return std::floor(d) == d;
    }
    inline bool isNumberExceededIntLimit(ValueType::DoubleClass d)
    {
        static constexpr ValueType::DoubleClass intMaxAsDouble = static_cast<ValueType::DoubleClass>(std::numeric_limits<ValueType::IntClass>::max());
        static constexpr ValueType::DoubleClass intMinAsDouble = static_cast<ValueType::DoubleClass>(std::numeric_limits<ValueType::IntClass>::min());
        return d > intMaxAsDouble || d < intMinAsDouble;
    }
    class Value
    {
    public:
        using VariantType = std::variant<Null, Int, Double, String, Bool, Function, StructType, StructInstance>;
        VariantType data;

        Value() :
            data(Null{}) {}
        Value(const Null &n) :
            data(std::in_place_type<Null>, n) {}
        Value(const Int &i) :
            data(std::in_place_type<Int>, i) {}
        Value(const Double &d) :
            data(std::in_place_type<Double>, d)
        {
            ValueType::IntClass casted = static_cast<ValueType::IntClass>(d.getValue());
            if (casted == d.getValue())
            {
                data.emplace<Int>(casted);
            }
        }
        Value(const String &s) :
            data(std::in_place_type<String>, s) {}
        Value(const Bool &b) :
            data(std::in_place_type<Bool>, b) {}
        Value(const Function &f) :
            data(std::in_place_type<Function>, f) {}
        Value(const StructType &s) :
            data(std::in_place_type<StructType>, s) {}
        Value(const StructInstance &s) :
            data(std::in_place_type<StructInstance>, s) {}

        template <typename T,
                  typename = std::enable_if_t<
                      std::is_same_v<T, ValueType::IntClass>
                      || std::is_same_v<T, ValueType::DoubleClass>
                      || std::is_same_v<T, ValueType::StringClass>
                      || std::is_same_v<T, ValueType::BoolClass>
                      || std::is_same_v<T, ValueType::FunctionClass>
                      || std::is_same_v<T, ValueType::StructTypeClass>>>
        Value(const T &val)
        {
            // 不可以用 data = 的形式
            // __ValueWrapper 构造、拷贝有限制
            if constexpr (std::is_same_v<T, ValueType::IntClass>)
                data.emplace<Int>(val);
            else if constexpr (std::is_same_v<T, ValueType::DoubleClass>)
            {
                ValueType::IntClass casted = static_cast<ValueType::IntClass>(val);
                if (casted == val)
                {
                    data.emplace<Int>(casted);
                }
                else
                {
                    data.emplace<Double>(val);
                }
            }
            else if constexpr (std::is_same_v<T, ValueType::StringClass>)
                data.emplace<String>(val);
            else if constexpr (std::is_same_v<T, ValueType::BoolClass>)
                data.emplace<Bool>(val);
            else if constexpr (std::is_same_v<T, ValueType::FunctionClass>)
                data.emplace<Function>(val);
            else if constexpr (std::is_same_v<T, ValueType::StructTypeClass>)
                data.emplace<StructType>(val);
            else if constexpr (std::is_same_v<T, ValueType::StructInstanceClass>)
                data.emplace<StructInstance>(val);
        }
        Value(const Value &) = default;
        Value(Value &&) noexcept = default;
        Value &operator=(const Value &) = default;
        Value &operator=(Value &&) noexcept = default;

        static Value defaultValue(TypeInfo ti)
        {
            if (ti == ValueType::Int)
                return Value(Int(0));
            else if (ti == ValueType::Double)
                return Value(Double(0.0));
            else if (ti == ValueType::String)
                return Value(String(u8""));
            else if (ti == ValueType::Bool)
                return Value(Bool(false));
            else if (ti == ValueType::Function)
                return getNullInstance();
            else if (ti == ValueType::StructType)
                return getNullInstance();
            else if (ti == ValueType::StructInstance)
                return getNullInstance();
            else
                return getNullInstance();
        }

        template <typename T>
        bool is() const
        {
            return std::holds_alternative<T>(data);
        }

        template <typename T>
        T &as()
        {
            return std::get<T>(data);
        }

        template <typename T>
        const T &as() const
        {
            return std::get<T>(data);
        }

        static Value getNullInstance()
        {
            static Value v(Null{});
            return v;
        }

        TypeInfo getTypeInfo() const
        {
            return std::visit([](auto &&val) { return val.ti; }, data);
        }

        bool isNull() const { return is<Null>(); }
        bool isNumeric() const { return is<Int>() || is<Double>(); }

        ValueType::DoubleClass getNumericValue() const
        {
            if (is<Int>())
                return static_cast<ValueType::DoubleClass>(as<Int>().getValue());
            else if (is<Double>())
                return as<Double>().getValue();
            else
                throw RuntimeError(u8"getNumericValue: Not a numeric value");
        }

        FString toString() const
        {
            if (is<Null>()) return FString(u8"null");
            if (is<Int>()) return FString(std::to_string(as<Int>().getValue()));
            if (is<Double>()) return FString(std::to_string(as<Double>().getValue()));
            if (is<String>()) return as<String>().getValue();
            if (is<Bool>()) return as<Bool>().getValue() ? FString(u8"true") : FString(u8"false");
            if (is<Function>())
            {
                return FString(std::format("<Function {} at {:p}>",
                                           as<Function>().getValue().id,
                                           static_cast<const void *>(as<Function>().data.get())));
            }
            if (is<StructType>())
            {
                return FString(std::format("<StructType {} at {:p}>",
                                           as<StructType>().getValue().id,
                                           static_cast<const void *>(as<StructType>().data.get())));
            }
            if (is<StructInstance>())
            {
                return FString(std::format("<Struct Instance('{}') at {:p}",
                                           as<StructInstance>().getValue().parentId,
                                           static_cast<const void *>(as<StructInstance>().data.get())));
            }
            return FString(u8"<error>");
        }

    private:
        static std::string makeTypeErrorMessage(const char *prefix, const char *op,
                                                const Value &lhs, const Value &rhs)
        {
            auto lhs_type = std::visit([](auto &&v) { return v.ti.name.toBasicString(); }, lhs.data);
            auto rhs_type = std::visit([](auto &&v) { return v.ti.name.toBasicString(); }, rhs.data);
            return std::format("{}: {} '{}' {}", prefix, lhs_type, op, rhs_type);
        }

    public:
        // math
        friend Value operator+(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot add", "+", lhs, rhs)));
            if (lhs.isNumeric() and rhs.isNumeric())
            {
                ValueType::DoubleClass result = lhs.getNumericValue() + rhs.getNumericValue();
                if (isDoubleInteger(result) and !isNumberExceededIntLimit(result))
                {
                    return Value(static_cast<ValueType::IntClass>(result));
                }
                return Value(ValueType::DoubleClass(result));
            }
            if (lhs.is<String>() && rhs.is<String>())
                return Value(ValueType::StringClass(lhs.as<String>().getValue() + rhs.as<String>().getValue()));

            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "+", lhs, rhs)));
        }

        friend Value operator-(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot subtract", "-", lhs, rhs)));
            if (lhs.isNumeric() and rhs.isNumeric())
            {
                ValueType::DoubleClass result = lhs.getNumericValue() - rhs.getNumericValue();
                if (isDoubleInteger(result) and !isNumberExceededIntLimit(result))
                {
                    return Value(static_cast<ValueType::IntClass>(result));
                }
                return Value(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "-", lhs, rhs)));
        }

        friend Value operator*(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot multiply", "*", lhs, rhs)));
            if (lhs.isNumeric() and rhs.isNumeric())
            {
                ValueType::DoubleClass result = lhs.getNumericValue() * rhs.getNumericValue();
                if (isDoubleInteger(result) and !isNumberExceededIntLimit(result))
                {
                    return Value(static_cast<ValueType::IntClass>(result));
                }
                return Value(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "*", lhs, rhs)));
        }

        friend Value operator/(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot divide", "/", lhs, rhs)));
            if (lhs.isNumeric() and rhs.isNumeric())
            {
                ValueType::DoubleClass rnv = rhs.getNumericValue();
                if (rnv == 0) throw ValueError(FStringView(makeTypeErrorMessage("Division by zero", "/", lhs, rhs)));
                ValueType::DoubleClass result = lhs.getNumericValue() / rnv;
                if (isDoubleInteger(result) and !isNumberExceededIntLimit(result))
                {
                    return Value(static_cast<ValueType::IntClass>(result));
                }
                return Value(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "/", lhs, rhs)));
        }

        friend Value operator%(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot modulo", "%", lhs, rhs)));
            if (lhs.isNumeric() and rhs.isNumeric())
            {
                ValueType::DoubleClass rnv = rhs.getNumericValue();
                if (rnv == 0) throw ValueError(FStringView(makeTypeErrorMessage("Modulo by zero", "/", lhs, rhs)));
                ValueType::DoubleClass result = fmod(lhs.getNumericValue(), rnv);
                if (isDoubleInteger(result) and !isNumberExceededIntLimit(result))
                {
                    return Value(static_cast<ValueType::IntClass>(result));
                }
                return Value(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "%", lhs, rhs)));
        }

        // logic
        friend Value operator&&(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Bool>() || !rhs.is<Bool>())
                throw ValueError(FStringView(makeTypeErrorMessage("Logical AND requires bool", "&&", lhs, rhs)));
            return Value(lhs.as<Bool>().getValue() && rhs.as<Bool>().getValue());
        }

        friend Value operator||(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Bool>() || !rhs.is<Bool>())
                throw ValueError(FStringView(makeTypeErrorMessage("Logical OR requires bool", "||", lhs, rhs)));
            return Value(lhs.as<Bool>().getValue() || rhs.as<Bool>().getValue());
        }

        friend Value operator!(const Value &v)
        {
            if (!v.is<Bool>())
                throw ValueError(FStringView(std::format("Logical NOT requires bool: '{}'",
                                                         std::visit([](auto &&val) { return val.ti.name.toBasicString(); }, v.data))));
            return Value(!v.as<Bool>().getValue());
        }

        friend Value operator-(const Value &v)
        {
            if (v.isNull())
                throw ValueError(FStringView(std::format("Unary minus cannot be applied to null")));
            if (v.is<Int>())
                return Value(-v.as<Int>().getValue());
            if (v.is<Double>())
                return Value(-v.as<Double>().getValue());
            throw ValueError(FStringView(std::format("Unary minus requires int or double: '{}'",
                                                     std::visit([](auto &&val) { return val.ti.name.toBasicString(); }, v.data))));
        }
        friend Value operator~(const Value &v)
        {
            if (!v.is<Int>())
                throw ValueError(FStringView(std::format("Bitwise NOT requires int: '{}'",
                                                         std::visit([](auto &&val) { return val.ti.name.toBasicString(); }, v.data))));
            return Value(~v.as<Int>().getValue());
        }

        // compare → now returns bool
        friend bool operator==(const Value &lhs, const Value &rhs)
        {
            return lhs.data == rhs.data;
        }

        friend bool operator!=(const Value &lhs, const Value &rhs)
        {
            return !(lhs.data == rhs.data);
        }

        friend bool operator<(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNumeric() and rhs.isNumeric())
                return lhs.getNumericValue() < rhs.getNumericValue();
            if (lhs.is<String>() && rhs.is<String>()) return lhs.as<String>().getValue() < rhs.as<String>().getValue();
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported comparison", "<", lhs, rhs)));
        }

        friend bool operator<=(const Value &lhs, const Value &rhs)
        {
            return lhs == rhs or lhs < rhs;
        }

        friend bool operator>(const Value &lhs, const Value &rhs)
        {
            if (lhs.isNumeric() and rhs.isNumeric())
                return lhs.getNumericValue() > rhs.getNumericValue();
            if (lhs.is<String>() && rhs.is<String>()) return lhs.as<String>().getValue() > rhs.as<String>().getValue();
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported comparison", ">", lhs, rhs)));
        }

        friend bool operator>=(const Value &lhs, const Value &rhs)
        {
            return lhs == rhs or lhs > rhs;
        }

        // bitwise
        friend Value bit_and(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise AND requires int", "&", lhs, rhs)));
            return Value(lhs.as<Int>().getValue() & rhs.as<Int>().getValue());
        }

        friend Value bit_or(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise OR requires int", "|", lhs, rhs)));
            return Value(lhs.as<Int>().getValue() | rhs.as<Int>().getValue());
        }

        friend Value bit_xor(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise XOR requires int", "^", lhs, rhs)));
            return Value(lhs.as<Int>().getValue() ^ rhs.as<Int>().getValue());
        }

        friend Value bit_not(const Value &v)
        {
            if (!v.is<Int>())
                throw ValueError(FStringView(std::format("Bitwise NOT requires int: '{}'",
                                                         std::visit([](auto &&val) { return val.ti.name.toBasicString(); }, v.data))));
            return Value(~v.as<Int>().getValue());
        }

        friend Value shift_left(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Shift left requires int", "<<", lhs, rhs)));
            return Value(lhs.as<Int>().getValue() << rhs.as<Int>().getValue());
        }

        friend Value shift_right(const Value &lhs, const Value &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Shift right requires int", ">>", lhs, rhs)));
            return Value(lhs.as<Int>().getValue() >> rhs.as<Int>().getValue());
        }

        friend Value power(const Value &base, const Value &exp)
        {
            if (base.isNull() || exp.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot exponentiate", "**", base, exp)));
            if (base.isNumeric() and exp.isNumeric())
            {
                ValueType::DoubleClass result = std::pow(base.getNumericValue(), exp.getNumericValue());
                if (isDoubleInteger(result) and !isNumberExceededIntLimit(result))
                {
                    return Value(static_cast<ValueType::IntClass>(result));
                }
                return Value(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "**", base, exp)));
        }
    };

    using Any = Value;
} // namespace Fig
