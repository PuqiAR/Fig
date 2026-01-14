#pragma once

#include <Core/fig_string.hpp>
#include <Ast/Statements/StructDefSt.hpp>

#include <Value/Type.hpp>

#include <Context/context_forward.hpp>
#include <atomic>
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
            return am == AccessModifier::Public || am == AccessModifier::PublicConst;
        }
        bool isConst() const
        {
            return am == AccessModifier::Const || am == AccessModifier::PublicConst;
        }
    };

    struct StructType
    {
        TypeInfo type;
        ContextPtr defContext; // 定义时的上下文
        std::vector<Field> fields;

        bool builtin = false;

        // ===== Constructors =====
        StructType(TypeInfo _type, ContextPtr _defContext, std::vector<Field> _fields, bool _builtin = false) :
            type(std::move(_type)), defContext(std::move(_defContext)), fields(std::move(_fields)), builtin(_builtin) {}

        StructType(const StructType &other) = default;
        StructType(StructType &&) noexcept = default;
        StructType &operator=(const StructType &) = default;
        StructType &operator=(StructType &&) noexcept = default;

        // ===== Comparison =====
        bool operator==(const StructType &other) const noexcept
        {
            return type == other.type;
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

namespace std
{
    template <>
    struct hash<Fig::Field>
    {
        size_t operator()(const Fig::Field &f)
        {
            return std::hash<Fig::FString>{}(f.name);
        }
    };
}; // namespace std