#pragma once

#include <Value/BaseValue.hpp>
#include <context_forward.hpp>

namespace Fig
{
    struct StructInstance
    {
        size_t parentId;
        ContextPtr localContext;

        // ===== Constructors =====
        StructInstance(size_t _parentId, ContextPtr _localContext) :
            parentId(_parentId), localContext(std::move(_localContext)) {}

        StructInstance(const StructInstance &other) = default;
        StructInstance(StructInstance &&) noexcept = default;
        StructInstance &operator=(const StructInstance &) = default;
        StructInstance &operator=(StructInstance &&) noexcept = default;

        // ===== Comparison =====
        bool operator==(const StructInstance &other) const noexcept
        {
            return parentId == other.parentId && localContext == other.localContext;
        }
        bool operator!=(const StructInstance &other) const noexcept
        {
            return !(*this == other);
        }
    };
} // namespace Fig
