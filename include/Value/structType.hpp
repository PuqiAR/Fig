#pragma once

#include <fig_string.hpp>
#include <Ast/StructDefSt.hpp>

#include <Value/Type.hpp>
#include <Value/BaseValue.hpp>

#include <context_forward.hpp>

#include <atomic>

namespace Fig
{
    struct Field
    {
        bool isPublic() const
        {
            return am == AccessModifier::Public or am == AccessModifier::PublicConst or am == AccessModifier::PublicFinal;
        }
        bool isConst() const
        {
            return am == AccessModifier::Const or am == AccessModifier::PublicConst;
        }
        bool isFinal() const
        {
            return am == AccessModifier::Final or am == AccessModifier::PublicFinal;
        }

        AccessModifier am;
        FString name;
        TypeInfo type;
        Ast::Expression defaultValue;

        Field(AccessModifier _am, FString _name, TypeInfo _type, Ast::Expression _defaultValue) :
            am(std::move(_am)), name(std::move(_name)), type(std::move(_type)), defaultValue(std::move(_defaultValue)) {}
    };
    struct StructT final// = StructType 结构体定义
    {
        std::size_t id;
        ContextPtr defContext; // 定义时的上下文
        std::vector<Field> fields;
        StructT(ContextPtr _defContext, std::vector<Field> fieldsMap) :
            defContext(std::move(_defContext)),
            fields(std::move(fieldsMap))
        {
            id = nextId();
        }
        StructT(const StructT &other) :
            id(other.id), fields(other.fields) {}
        StructT &operator=(const StructT &other) = default;
        StructT(StructT &&) noexcept = default;
        StructT &operator=(StructT &&) noexcept = default;

        bool operator==(const StructT &other) const noexcept
        {
            return id == other.id;
        }
        bool operator!=(const StructT &other) const noexcept
        {
            return !(*this == other);
        }

    private:
        static std::size_t nextId()
        {
            static std::atomic<std::size_t> counter{1};
            return counter++;
        }
    };

    class StructType final : public __ValueWrapper<StructT>
    {
    public:
        StructType(const StructT &x) :
            __ValueWrapper(ValueType::StructType)
        {
            data = std::make_unique<StructT>(x);
        }
        StructType(ContextPtr _defContext, std::vector<Field> fieldsMap) :
            __ValueWrapper(ValueType::StructType)
        {
            data = std::make_unique<StructT>(std::move(_defContext), std::move(fieldsMap));
        }
        bool operator==(const StructType &other) const noexcept
        {
            return data == other.data;
        }
        StructType(const StructType &) = default;
        StructType(StructType &&) noexcept = default;
        StructType &operator=(const StructType &) = default;
        StructType &operator=(StructType &&) noexcept = default;
    };

}; // namespace Fig