#pragma once

#include <Context/context_forward.hpp>
#include <Value/Type.hpp>

namespace Fig
{
    struct StructInstance
    {
        TypeInfo parentType;
        ContextPtr localContext;

        // ===== Constructors =====
        StructInstance(TypeInfo _parentType, ContextPtr _localContext) :
            parentType(_parentType), localContext(std::move(_localContext)) {}

        StructInstance(const StructInstance &other) = default;
        StructInstance(StructInstance &&) noexcept = default;
        StructInstance &operator=(const StructInstance &) = default;
        StructInstance &operator=(StructInstance &&) noexcept = default;

        // ===== Comparison =====
        bool operator==(const StructInstance &other) const noexcept
        {
            return parentType == other.parentType && localContext == other.localContext;
        }
        bool operator!=(const StructInstance &other) const noexcept
        {
            return !(*this == other);
        }
    };
} // namespace Fig
