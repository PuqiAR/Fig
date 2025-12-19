#pragma once

#include <Value/BaseValue.hpp>
#include <Value/Type.hpp>
#include <Ast/functionParameters.hpp>

#include <atomic>
namespace Fig
{
    /* complex objects */
    struct FunctionStruct
    {
        std::size_t id;
        Ast::FunctionParameters paras;
        TypeInfo retType;
        Ast::BlockStatement body;

        FunctionStruct(Ast::FunctionParameters _paras, TypeInfo _retType, Ast::BlockStatement _body) :
            id(nextId()), // 分配唯一 ID
            paras(std::move(_paras)),
            retType(std::move(_retType)),
            body(std::move(_body))
        {
        }

        FunctionStruct(const FunctionStruct &other) :
            id(other.id), paras(other.paras), retType(other.retType), body(other.body) {}

        FunctionStruct &operator=(const FunctionStruct &other) = default;
        FunctionStruct(FunctionStruct &&) noexcept = default;
        FunctionStruct &operator=(FunctionStruct &&) noexcept = default;

        bool operator==(const FunctionStruct &other) const noexcept
        {
            return id == other.id;
        }
        bool operator!=(const FunctionStruct &other) const noexcept
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

    class Function final : public __ValueWrapper<FunctionStruct>
    {
    public:
        Function(const FunctionStruct &x) :
            __ValueWrapper(ValueType::Function)
        {
            data = std::make_unique<FunctionStruct>(x);
        }
        Function(Ast::FunctionParameters paras, TypeInfo ret, Ast::BlockStatement body) :
            __ValueWrapper(ValueType::Function)
        {
            data = std::make_unique<FunctionStruct>(
                std::move(paras), std::move(ret), std::move(body));
        }
        bool operator==(const Function &other) const noexcept
        {
            if (!data || !other.data) return false;
            return *data == *other.data; // call -> FunctionStruct::operator== (based on ID comparing)
        }
        Function(const Function &) = default;
        Function(Function &&) noexcept = default;
        Function &operator=(const Function &) = default;
        Function &operator=(Function &&) noexcept = default;
    };
} // namespace Fig
