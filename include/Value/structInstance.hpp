#pragma once

#include <Value/BaseValue.hpp>

#include <context_forward.hpp>

namespace Fig
{
    struct StructInstanceT final
    {
        FString structName; // 类的名字 (StructType), 非变量名。用于获取所属类
        ContextPtr localContext;

        StructInstanceT(FString _structName, ContextPtr _localContext) :
            structName(std::move(_structName)), localContext(std::move(_localContext)) {}

        StructInstanceT(const StructInstanceT &other) :
            structName(other.structName), localContext(other.localContext) {}
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
        StructInstance(FString _structName, ContextPtr _localContext) :
            __ValueWrapper(ValueType::StructInstance)
        {
            data = std::make_unique<StructInstanceT>(std::move(_structName), std::move(_localContext));
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