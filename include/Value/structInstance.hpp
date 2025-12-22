#pragma once

#include <Value/BaseValue.hpp>

#include <context_forward.hpp>

namespace Fig
{
    struct StructInstanceT final
    {
        size_t parentId;
        ContextPtr localContext;

        StructInstanceT(size_t _parentId, ContextPtr _localContext) :
            parentId(std::move(_parentId)), localContext(std::move(_localContext)) {}

        StructInstanceT(const StructInstanceT &other) :
            parentId(other.parentId), localContext(other.localContext) {}
        StructInstanceT &operator=(const StructInstanceT &) = default;
        StructInstanceT(StructInstanceT &&) = default;
        StructInstanceT &operator=(StructInstanceT &&) = default;

        bool operator==(const StructInstanceT &) const = default;
    };

    class StructInstance final : public __ValueWrapper<StructInstanceT>
    {
    public:
        StructInstance(const StructInstanceT &x) :
            __ValueWrapper(ValueType::StructInstance)
        {
            data = std::make_unique<StructInstanceT>(x);
        }
        StructInstance(size_t _parentId, ContextPtr _localContext) :
            __ValueWrapper(ValueType::StructInstance)
        {
            data = std::make_unique<StructInstanceT>(std::move(_parentId), std::move(_localContext));
        }

        bool operator==(const StructInstance &other) const noexcept
        {
            return data == other.data;
        }
        StructInstance(const StructInstance &) = default;
        StructInstance(StructInstance &&) = default;
        StructInstance &operator=(const StructInstance &) = default;
        StructInstance &operator=(StructInstance &&) = default;
    };

}; // namespace Fig