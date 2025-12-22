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

    class Object
    {
    public:
        using VariantType = std::variant<
            Null,
            Int,
            Double,
            String,
            Bool,
            Function,
            StructType,
            StructInstance>;

        VariantType data;

        Object() :
            data(Null{}) {}
        Object(const Null &n) :
            data(std::in_place_type<Null>, n) {}
        Object(const Int &i) :
            data(std::in_place_type<Int>, i) {}
        Object(const Double &d)
        {
            ValueType::IntClass casted = static_cast<ValueType::IntClass>(d.getValue());
            if (casted == d.getValue())
                data.emplace<Int>(casted);
            else
                data.emplace<Double>(d);
        }
        Object(const String &s) :
            data(std::in_place_type<String>, s) {}
        Object(const Bool &b) :
            data(std::in_place_type<Bool>, b) {}
        Object(const Function &f) :
            data(std::in_place_type<Function>, f) {}
        Object(const StructType &s) :
            data(std::in_place_type<StructType>, s) {}
        Object(const StructInstance &s) :
            data(std::in_place_type<StructInstance>, s) {}

        template <typename T,
                  typename = std::enable_if_t<
                      std::is_same_v<T, ValueType::IntClass>
                      || std::is_same_v<T, ValueType::DoubleClass>
                      || std::is_same_v<T, ValueType::StringClass>
                      || std::is_same_v<T, ValueType::BoolClass>
                      || std::is_same_v<T, Function>
                      || std::is_same_v<T, StructType>
                      || std::is_same_v<T, StructInstance>>>
        Object(const T &val)
        {
            if constexpr (std::is_same_v<T, ValueType::IntClass>)
                data.emplace<Int>(val);
            else if constexpr (std::is_same_v<T, ValueType::DoubleClass>)
            {
                ValueType::IntClass casted = static_cast<ValueType::IntClass>(val);
                if (casted == val)
                    data.emplace<Int>(casted);
                else
                    data.emplace<Double>(val);
            }
            else if constexpr (std::is_same_v<T, ValueType::StringClass>)
                data.emplace<String>(val);
            else if constexpr (std::is_same_v<T, ValueType::BoolClass>)
                data.emplace<Bool>(val);
            else if constexpr (std::is_same_v<T, Function>)
                data.emplace<Function>(val);
            else if constexpr (std::is_same_v<T, StructType>)
                data.emplace<StructType>(val);
            else if constexpr (std::is_same_v<T, StructInstance>)
                data.emplace<StructInstance>(val);
        }

        Object(const Object &) = default;
        Object(Object &&) noexcept = default;
        Object &operator=(const Object &) = default;
        Object &operator=(Object &&) noexcept = default;

        static Object defaultValue(TypeInfo ti)
        {
            if (ti == ValueType::Int)
                return Object(Int(0));
            else if (ti == ValueType::Double)
                return Object(Double(0.0));
            else if (ti == ValueType::String)
                return Object(String(u8""));
            else if (ti == ValueType::Bool)
                return Object(Bool(false));
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

        static Object getNullInstance()
        {
            static Object v(Null{});
            return v;
        }

        TypeInfo getTypeInfo() const
        {
            return std::visit([](auto &&val) -> TypeInfo {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, Null>)
                    return ValueType::Null;
                else if constexpr (std::is_same_v<T, Int>)
                    return ValueType::Int;
                else if constexpr (std::is_same_v<T, Double>)
                    return ValueType::Double;
                else if constexpr (std::is_same_v<T, String>)
                    return ValueType::String;
                else if constexpr (std::is_same_v<T, Bool>)
                    return ValueType::Bool;
                else if constexpr (std::is_same_v<T, Function>)
                    return ValueType::Function;
                else if constexpr (std::is_same_v<T, StructType>)
                    return ValueType::StructType;
                else if constexpr (std::is_same_v<T, StructInstance>)
                    return ValueType::StructInstance;
                else
                    return ValueType::Any;
            },
                              data);
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
                return FString(std::format("<Function {} at {:p}>",
                                           as<Function>().id,
                                           static_cast<const void *>(&as<Function>())));
            if (is<StructType>())
                return FString(std::format("<StructType {} at {:p}>",
                                           as<StructType>().id,
                                           static_cast<const void *>(&as<StructType>())));
            if (is<StructInstance>())
                return FString(std::format("<Struct Instance('{}') at {:p}>",
                                           as<StructInstance>().parentId,
                                           static_cast<const void *>(&as<StructInstance>())));
            return FString(u8"<error>");
        }

    private:
        static std::string makeTypeErrorMessage(const char *prefix, const char *op,
                                                const Object &lhs, const Object &rhs)
        {
            auto lhs_type = lhs.getTypeInfo().name.toBasicString();
            auto rhs_type = rhs.getTypeInfo().name.toBasicString();
            return std::format("{}: {} '{}' {}", prefix, lhs_type, op, rhs_type);
        }

    public:
        // math
        friend Object operator+(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot add", "+", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                bool lhsIsInt = lhs.is<Int>();
                bool rhsIsInt = rhs.is<Int>();
                ValueType::DoubleClass result = lhs.getNumericValue() + rhs.getNumericValue();
                if (lhsIsInt && rhsIsInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(ValueType::DoubleClass(result));
            }
            if (lhs.is<String>() && rhs.is<String>())
                return Object(ValueType::StringClass(lhs.as<String>().getValue() + rhs.as<String>().getValue()));
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "+", lhs, rhs)));
        }

        friend Object operator-(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot subtract", "-", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                bool lhsIsInt = lhs.is<Int>();
                bool rhsIsInt = rhs.is<Int>();
                ValueType::DoubleClass result = lhs.getNumericValue() - rhs.getNumericValue();
                if (lhsIsInt && rhsIsInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "-", lhs, rhs)));
        }

        friend Object operator*(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot multiply", "*", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                bool lhsIsInt = lhs.is<Int>();
                bool rhsIsInt = rhs.is<Int>();
                ValueType::DoubleClass result = lhs.getNumericValue() * rhs.getNumericValue();
                if (lhsIsInt && rhsIsInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "*", lhs, rhs)));
        }

        friend Object operator/(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot divide", "/", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                ValueType::DoubleClass rnv = rhs.getNumericValue();
                if (rnv == 0)
                    throw ValueError(FStringView(makeTypeErrorMessage("Division by zero", "/", lhs, rhs)));
                ValueType::DoubleClass result = lhs.getNumericValue() / rnv;
                bool lhsIsInt = lhs.is<Int>();
                bool rhsIsInt = rhs.is<Int>();
                if (lhsIsInt && rhsIsInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "/", lhs, rhs)));
        }

        friend Object operator%(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot modulo", "%", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                ValueType::DoubleClass rnv = rhs.getNumericValue();
                if (rnv == 0)
                    throw ValueError(FStringView(makeTypeErrorMessage("Modulo by zero", "%", lhs, rhs)));
                ValueType::DoubleClass result = fmod(lhs.getNumericValue(), rnv);
                bool lhsIsInt = lhs.is<Int>();
                bool rhsIsInt = rhs.is<Int>();
                if (lhsIsInt && rhsIsInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "%", lhs, rhs)));
        }

        // logic
        friend Object operator&&(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Bool>() || !rhs.is<Bool>())
                throw ValueError(FStringView(makeTypeErrorMessage("Logical AND requires bool", "&&", lhs, rhs)));
            return Object(lhs.as<Bool>().getValue() && rhs.as<Bool>().getValue());
        }

        friend Object operator||(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Bool>() || !rhs.is<Bool>())
                throw ValueError(FStringView(makeTypeErrorMessage("Logical OR requires bool", "||", lhs, rhs)));
            return Object(lhs.as<Bool>().getValue() || rhs.as<Bool>().getValue());
        }

        friend Object operator!(const Object &v)
        {
            if (!v.is<Bool>())
                throw ValueError(FStringView(std::format("Logical NOT requires bool: '{}'", v.getTypeInfo().name.toBasicString())));
            return Object(!v.as<Bool>().getValue());
        }

        friend Object operator-(const Object &v)
        {
            if (v.isNull())
                throw ValueError(FStringView(std::format("Unary minus cannot be applied to null")));
            if (v.is<Int>())
                return Object(-v.as<Int>().getValue());
            if (v.is<Double>())
                return Object(-v.as<Double>().getValue());
            throw ValueError(FStringView(std::format("Unary minus requires int or double: '{}'", v.getTypeInfo().name.toBasicString())));
        }

        friend Object operator~(const Object &v)
        {
            if (!v.is<Int>())
                throw ValueError(FStringView(std::format("Bitwise NOT requires int: '{}'", v.getTypeInfo().name.toBasicString())));
            return Object(~v.as<Int>().getValue());
        }

        // compare
        friend bool operator==(const Object &lhs, const Object &rhs)
        {
            return lhs.data == rhs.data;
        }

        friend bool operator!=(const Object &lhs, const Object &rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator<(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNumeric() && rhs.isNumeric())
                return lhs.getNumericValue() < rhs.getNumericValue();
            if (lhs.is<String>() && rhs.is<String>())
                return lhs.as<String>().getValue() < rhs.as<String>().getValue();
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported comparison", "<", lhs, rhs)));
        }

        friend bool operator<=(const Object &lhs, const Object &rhs)
        {
            return lhs == rhs || lhs < rhs;
        }

        friend bool operator>(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNumeric() && rhs.isNumeric())
                return lhs.getNumericValue() > rhs.getNumericValue();
            if (lhs.is<String>() && rhs.is<String>())
                return lhs.as<String>().getValue() > rhs.as<String>().getValue();
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported comparison", ">", lhs, rhs)));
        }

        friend bool operator>=(const Object &lhs, const Object &rhs)
        {
            return lhs == rhs || lhs > rhs;
        }

        // bitwise
        friend Object bit_and(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise AND requires int", "&", lhs, rhs)));
            return Object(lhs.as<Int>().getValue() & rhs.as<Int>().getValue());
        }

        friend Object bit_or(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise OR requires int", "|", lhs, rhs)));
            return Object(lhs.as<Int>().getValue() | rhs.as<Int>().getValue());
        }

        friend Object bit_xor(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise XOR requires int", "^", lhs, rhs)));
            return Object(lhs.as<Int>().getValue() ^ rhs.as<Int>().getValue());
        }

        friend Object bit_not(const Object &v)
        {
            if (!v.is<Int>())
                throw ValueError(FStringView(std::format("Bitwise NOT requires int: '{}'", v.getTypeInfo().name.toBasicString())));
            return Object(~v.as<Int>().getValue());
        }

        friend Object shift_left(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Shift left requires int", "<<", lhs, rhs)));
            return Object(lhs.as<Int>().getValue() << rhs.as<Int>().getValue());
        }

        friend Object shift_right(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<Int>() || !rhs.is<Int>())
                throw ValueError(FStringView(makeTypeErrorMessage("Shift right requires int", ">>", lhs, rhs)));
            return Object(lhs.as<Int>().getValue() >> rhs.as<Int>().getValue());
        }

        friend Object power(const Object &base, const Object &exp)
        {
            if (base.isNull() || exp.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot exponentiate", "**", base, exp)));
            if (base.isNumeric() && exp.isNumeric())
            {
                bool baseIsInt = base.is<Int>();
                bool expIsInt = exp.is<Int>();
                ValueType::DoubleClass result = std::pow(base.getNumericValue(), exp.getNumericValue());
                if (baseIsInt && expIsInt && isDoubleInteger(result) && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(ValueType::DoubleClass(result));
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "**", base, exp)));
        }
    };

    using Any = Object;
} // namespace Fig
