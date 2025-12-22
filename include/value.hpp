#pragma once
#include <Value/function.hpp>
#include <Value/structType.hpp>
#include <Value/structInstance.hpp>
#include <Value/Type.hpp>
#include <Value/valueError.hpp>

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
            ValueType::NullClass,
            ValueType::IntClass,
            ValueType::DoubleClass,
            ValueType::StringClass,
            ValueType::BoolClass,
            Function,
            StructType,
            StructInstance>;

        VariantType data;

        Object() :
            data(ValueType::NullClass{}) {}
        Object(const ValueType::NullClass &n) :
            data(n) {}
        Object(const ValueType::IntClass &i) :
            data(i) {}
        Object(const ValueType::DoubleClass &d)
        {
            ValueType::IntClass casted = static_cast<ValueType::IntClass>(d);
            if (casted == d)
                data = casted;
            else
                data = d;
        }
        Object(const ValueType::StringClass &s) :
            data(s) {}
        Object(const ValueType::BoolClass &b) :
            data(b) {}
        Object(const Function &f) :
            data(f) {}
        Object(const StructType &s) :
            data(s) {}
        Object(const StructInstance &s) :
            data(s) {}

        Object(const Object &) = default;
        Object(Object &&) noexcept = default;
        Object &operator=(const Object &) = default;
        Object &operator=(Object &&) noexcept = default;

        static Object defaultValue(TypeInfo ti)
        {
            if (ti == ValueType::Int)
                return Object(ValueType::IntClass(0));
            else if (ti == ValueType::Double)
                return Object(ValueType::DoubleClass(0.0));
            else if (ti == ValueType::String)
                return Object(ValueType::StringClass(u8""));
            else if (ti == ValueType::Bool)
                return Object(ValueType::BoolClass(false));
            else
                return *getNullInstance();
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

        static std::shared_ptr<Object> getNullInstance()
        {
            static std::shared_ptr<Object> n = std::make_shared<Object>(ValueType::NullClass{});
            return n;
        }
        static std::shared_ptr<Object> getTrueInstance()
        {
            static std::shared_ptr<Object> t = std::make_shared<Object>(true);
            return t;
        }
        static std::shared_ptr<Object> getFalseInstance()
        {
            static std::shared_ptr<Object> f = std::make_shared<Object>(false);
            return f;
        }

        TypeInfo getTypeInfo() const
        {
            return std::visit([](auto &&val) -> TypeInfo {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, ValueType::NullClass>)
                    return ValueType::Null;
                else if constexpr (std::is_same_v<T, ValueType::IntClass>)
                    return ValueType::Int;
                else if constexpr (std::is_same_v<T, ValueType::DoubleClass>)
                    return ValueType::Double;
                else if constexpr (std::is_same_v<T, ValueType::StringClass>)
                    return ValueType::String;
                else if constexpr (std::is_same_v<T, ValueType::BoolClass>)
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

        bool isNull() const { return is<ValueType::NullClass>(); }
        bool isNumeric() const { return is<ValueType::IntClass>() || is<ValueType::DoubleClass>(); }

        ValueType::DoubleClass getNumericValue() const
        {
            if (is<ValueType::IntClass>())
                return static_cast<ValueType::DoubleClass>(as<ValueType::IntClass>());
            else if (is<ValueType::DoubleClass>())
                return as<ValueType::DoubleClass>();
            else
                throw RuntimeError(u8"getNumericValue: Not a numeric value");
        }

        FString toString() const
        {
            if (is<ValueType::NullClass>()) return FString(u8"null");
            if (is<ValueType::IntClass>()) return FString(std::to_string(as<ValueType::IntClass>()));
            if (is<ValueType::DoubleClass>()) return FString(std::format("{}", as<ValueType::DoubleClass>()));
            if (is<ValueType::StringClass>()) return as<ValueType::StringClass>();
            if (is<ValueType::BoolClass>()) return as<ValueType::BoolClass>() ? FString(u8"true") : FString(u8"false");
            if (is<Function>())
                return FString(std::format("<Function {} at {:p}>",
                                           as<Function>().id,
                                           static_cast<const void *>(&as<Function>())));
            if (is<StructType>())
                return FString(std::format("<StructType {} at {:p}>",
                                           as<StructType>().id,
                                           static_cast<const void *>(&as<StructType>())));
            if (is<StructInstance>())
                return FString(std::format("<StructInstance '{}' at {:p}>",
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
                bool bothInt = lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>();
                auto result = lhs.getNumericValue() + rhs.getNumericValue();
                if (bothInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(result);
            }
            if (lhs.is<ValueType::StringClass>() && rhs.is<ValueType::StringClass>())
                return Object(FString(lhs.as<ValueType::StringClass>() + rhs.as<ValueType::StringClass>()));
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "+", lhs, rhs)));
        }

        friend Object operator-(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot subtract", "-", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                bool bothInt = lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>();
                auto result = lhs.getNumericValue() - rhs.getNumericValue();
                if (bothInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(result);
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "-", lhs, rhs)));
        }

        friend Object operator*(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot multiply", "*", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                bool bothInt = lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>();
                auto result = lhs.getNumericValue() * rhs.getNumericValue();
                if (bothInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(result);
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "*", lhs, rhs)));
        }

        friend Object operator/(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot divide", "/", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                auto rnv = rhs.getNumericValue();
                if (rnv == 0)
                    throw ValueError(FStringView(makeTypeErrorMessage("Division by zero", "/", lhs, rhs)));
                bool bothInt = lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>();
                auto result = lhs.getNumericValue() / rnv;
                if (bothInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(result);
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "/", lhs, rhs)));
        }

        friend Object operator%(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNull() || rhs.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot modulo", "%", lhs, rhs)));
            if (lhs.isNumeric() && rhs.isNumeric())
            {
                auto rnv = rhs.getNumericValue();
                if (rnv == 0)
                    throw ValueError(FStringView(makeTypeErrorMessage("Modulo by zero", "/", lhs, rhs)));
                bool bothInt = lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>();
                auto result = std::fmod(lhs.getNumericValue(), rnv);
                if (bothInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(result);
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "%", lhs, rhs)));
        }

        // logic
        friend Object operator&&(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::BoolClass>() || !rhs.is<ValueType::BoolClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Logical AND requires bool", "&&", lhs, rhs)));
            return Object(lhs.as<ValueType::BoolClass>() && rhs.as<ValueType::BoolClass>());
        }

        friend Object operator||(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::BoolClass>() || !rhs.is<ValueType::BoolClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Logical OR requires bool", "||", lhs, rhs)));
            return Object(lhs.as<ValueType::BoolClass>() || rhs.as<ValueType::BoolClass>());
        }

        friend Object operator!(const Object &v)
        {
            if (!v.is<ValueType::BoolClass>())
                throw ValueError(FStringView(std::format("Logical NOT requires bool: '{}'", v.getTypeInfo().name.toBasicString())));
            return Object(!v.as<ValueType::BoolClass>());
        }

        friend Object operator-(const Object &v)
        {
            if (v.isNull())
                throw ValueError(FStringView(u8"Unary minus cannot be applied to null"));
            if (v.is<ValueType::IntClass>())
                return Object(-v.as<ValueType::IntClass>());
            if (v.is<ValueType::DoubleClass>())
                return Object(-v.as<ValueType::DoubleClass>());
            throw ValueError(FStringView(std::format("Unary minus requires int or double: '{}'", v.getTypeInfo().name.toBasicString())));
        }

        friend Object operator~(const Object &v)
        {
            if (!v.is<ValueType::IntClass>())
                throw ValueError(FStringView(std::format("Bitwise NOT requires int: '{}'", v.getTypeInfo().name.toBasicString())));
            return Object(~v.as<ValueType::IntClass>());
        }

        // comparison
        friend bool operator==(const Object &lhs, const Object &rhs) { return lhs.data == rhs.data; }
        friend bool operator!=(const Object &lhs, const Object &rhs) { return !(lhs == rhs); }
        friend bool operator<(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNumeric() && rhs.isNumeric()) return lhs.getNumericValue() < rhs.getNumericValue();
            if (lhs.is<ValueType::StringClass>() && rhs.is<ValueType::StringClass>())
                return lhs.as<ValueType::StringClass>() < rhs.as<ValueType::StringClass>();
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported comparison", "<", lhs, rhs)));
        }
        friend bool operator<=(const Object &lhs, const Object &rhs) { return lhs == rhs || lhs < rhs; }
        friend bool operator>(const Object &lhs, const Object &rhs)
        {
            if (lhs.isNumeric() && rhs.isNumeric()) return lhs.getNumericValue() > rhs.getNumericValue();
            if (lhs.is<ValueType::StringClass>() && rhs.is<ValueType::StringClass>())
                return lhs.as<ValueType::StringClass>() > rhs.as<ValueType::StringClass>();
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported comparison", ">", lhs, rhs)));
        }
        friend bool operator>=(const Object &lhs, const Object &rhs) { return lhs == rhs || lhs > rhs; }

        // bitwise
        friend Object bit_and(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::IntClass>() || !rhs.is<ValueType::IntClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise AND requires int", "&", lhs, rhs)));
            return Object(lhs.as<ValueType::IntClass>() & rhs.as<ValueType::IntClass>());
        }

        friend Object bit_or(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::IntClass>() || !rhs.is<ValueType::IntClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise OR requires int", "|", lhs, rhs)));
            return Object(lhs.as<ValueType::IntClass>() | rhs.as<ValueType::IntClass>());
        }

        friend Object bit_xor(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::IntClass>() || !rhs.is<ValueType::IntClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Bitwise XOR requires int", "^", lhs, rhs)));
            return Object(lhs.as<ValueType::IntClass>() ^ rhs.as<ValueType::IntClass>());
        }

        friend Object bit_not(const Object &v)
        {
            if (!v.is<ValueType::IntClass>())
                throw ValueError(FStringView(std::format("Bitwise NOT requires int: '{}'", v.getTypeInfo().name.toBasicString())));
            return Object(~v.as<ValueType::IntClass>());
        }

        friend Object shift_left(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::IntClass>() || !rhs.is<ValueType::IntClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Shift left requires int", "<<", lhs, rhs)));
            return Object(lhs.as<ValueType::IntClass>() << rhs.as<ValueType::IntClass>());
        }

        friend Object shift_right(const Object &lhs, const Object &rhs)
        {
            if (!lhs.is<ValueType::IntClass>() || !rhs.is<ValueType::IntClass>())
                throw ValueError(FStringView(makeTypeErrorMessage("Shift right requires int", ">>", lhs, rhs)));
            return Object(lhs.as<ValueType::IntClass>() >> rhs.as<ValueType::IntClass>());
        }

        friend Object power(const Object &base, const Object &exp)
        {
            if (base.isNull() || exp.isNull())
                throw ValueError(FStringView(makeTypeErrorMessage("Cannot exponentiate", "**", base, exp)));
            if (base.isNumeric() && exp.isNumeric())
            {
                bool bothInt = base.is<ValueType::IntClass>() && exp.is<ValueType::IntClass>();
                auto result = std::pow(base.getNumericValue(), exp.getNumericValue());
                if (bothInt && !isNumberExceededIntLimit(result))
                    return Object(static_cast<ValueType::IntClass>(result));
                return Object(result);
            }
            throw ValueError(FStringView(makeTypeErrorMessage("Unsupported operation", "**", base, exp)));
        }
    };

    using ObjectPtr = std::shared_ptr<Object>;

} // namespace Fig
