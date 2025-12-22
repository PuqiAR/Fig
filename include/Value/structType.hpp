#pragma once

#include <fig_string.hpp>
#include <Ast/StructDefSt.hpp>

#include <Value/Type.hpp>
#include <Value/BaseValue.hpp>

#include <context_forward.hpp>
#include <atomic>
#include <memory>
#include <vector>

namespace Fig
{
    struct Field
    {
        AccessModifier am;
        FString name;
        TypeInfo type;
        Ast::Expression defaultValue;

        Field(AccessModifier _am, FString _name, TypeInfo _type, Ast::Expression _defaultValue) :
            am(_am), name(std::move(_name)), type(std::move(_type)), defaultValue(std::move(_defaultValue)) {}

        bool isPublic() const
        {
            return am == AccessModifier::Public || am == AccessModifier::PublicConst || am == AccessModifier::PublicFinal;
        }
        bool isConst() const
        {
            return am == AccessModifier::Const || am == AccessModifier::PublicConst;
        }
        bool isFinal() const
        {
            return am == AccessModifier::Final || am == AccessModifier::PublicFinal;
        }
    };

    struct StructType
    {
        std::size_t id;
        ContextPtr defContext; // 定义时的上下文
        std::vector<Field> fields;

        // ===== Constructors =====
        StructType(ContextPtr _defContext, std::vector<Field> _fields) :
            id(nextId()), defContext(std::move(_defContext)), fields(std::move(_fields)) {}

        StructType(const StructType &other) = default;
        StructType(StructType &&) noexcept = default;
        StructType &operator=(const StructType &) = default;
        StructType &operator=(StructType &&) noexcept = default;

        // ===== Comparison =====
        bool operator==(const StructType &other) const noexcept
        {
            return id == other.id;
        }
        bool operator!=(const StructType &other) const noexcept
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
} // namespace Fig
