#pragma once

#include <Value/Type.hpp>
#include <fig_string.hpp>

#include <memory>
#include <format>

namespace Fig
{

    template <class T>
    class __ValueWrapper
    {
    public:
        const TypeInfo ti;
        std::unique_ptr<T> data;

        __ValueWrapper(const __ValueWrapper &other) :
            ti(other.ti)
        {
            if (other.data)
                data = std::make_unique<T>(*other.data);
        }

        __ValueWrapper(__ValueWrapper &&other) noexcept
            :
            ti(other.ti), data(std::move(other.data)) {}

        __ValueWrapper &operator=(const __ValueWrapper &other)
        {
            if (this != &other)
            {
                if (other.data)
                    data = std::make_unique<T>(*other.data);
                else
                    data.reset();
            }
            return *this;
        }

        __ValueWrapper &operator=(__ValueWrapper &&other) noexcept
        {
            if (this != &other)
            {
                data = std::move(other.data);
            }
            return *this;
        }
        
        const T &getValue() const
        {
            if (!data) throw std::runtime_error("Accessing null Value data");
            return *data;
        }

        virtual size_t getSize() const
        {
            return sizeof(T);
        }

        virtual bool isNull() const
        {
            return !data;
        }

        virtual FString toString() const
        {
            if (!data)
                return FString(std::format("<{} object (null)>", ti.name.toBasicString()));

            return FString(std::format(
                "<{} object @{:p}>",
                ti.name.toBasicString(),
                static_cast<const void *>(data.get())));
        }

        __ValueWrapper(const TypeInfo &_ti) :
            ti(_ti) {}

        __ValueWrapper(const T &x, const TypeInfo &_ti) :
            ti(_ti)
        {
            data = std::make_unique<T>(x);
        }
    };

    class Int final : public __ValueWrapper<ValueType::IntClass>
    {
    public:
        Int(const ValueType::IntClass &x) :
            __ValueWrapper(x, ValueType::Int) {}

        Int(const Int &) = default;
        Int(Int &&) noexcept = default;

        Int &operator=(const Int &) = default;
        Int &operator=(Int &&) noexcept = default;

        bool operator==(const Int &other) const noexcept
        {
            return getValue() == other.getValue();
        }
        bool operator!=(const Int &other) const noexcept
        {
            return !(*this == other);
        }
    };

    class Double final : public __ValueWrapper<ValueType::DoubleClass>
    {
    public:
        Double(const ValueType::DoubleClass &x) :
            __ValueWrapper(x, ValueType::Double) {}
        Double(const Double &) = default;
        Double(Double &&) noexcept = default;

        Double &operator=(const Double &) = default;
        Double &operator=(Double &&) noexcept = default;

        bool operator==(const Double &other) const noexcept
        {
            return getValue() == other.getValue();
        }
        bool operator!=(const Double &other) const noexcept
        {
            return !(*this == other);
        }
    };

    class Null final : public __ValueWrapper<ValueType::NullClass>
    {
    public:
        Null() :
            __ValueWrapper(ValueType::NullClass{}, ValueType::Null) {}

        Null(const Null &) = default;
        Null(Null &&) noexcept = default;

        Null &operator=(const Null &) = default;
        Null &operator=(Null &&) noexcept = default;

        bool isNull() const override { return true; }
        bool operator==(const Null &) const noexcept { return true; }
        bool operator!=(const Null &) const noexcept { return false; }
    };

    class String final : public __ValueWrapper<ValueType::StringClass>
    {
    public:
        String(const ValueType::StringClass &x) :
            __ValueWrapper(x, ValueType::String) {}
        String(const String &) = default;
        String(String &&) noexcept = default;

        String &operator=(const String &) = default;
        String &operator=(String &&) noexcept = default;

        bool operator==(const String &other) const noexcept
        {
            return getValue() == other.getValue();
        }
        bool operator!=(const String &other) const noexcept
        {
            return !(*this == other);
        }
    };

    class Bool final : public __ValueWrapper<ValueType::BoolClass>
    {
    public:
        Bool(const ValueType::BoolClass &x) :
            __ValueWrapper(x, ValueType::Bool) {}

        Bool(const Bool &) = default;
        Bool(Bool &&) noexcept = default;

        Bool &operator=(const Bool &) = default;
        Bool &operator=(Bool &&) noexcept = default;

        bool operator==(const Bool &other) const noexcept
        {
            return getValue() == other.getValue();
        }
        bool operator!=(const Bool &other) const noexcept
        {
            return !(*this == other);
        }
    };
} // namespace Fig
