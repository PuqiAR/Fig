#pragma once

#include <Ast/functionParameters.hpp>
#include <Context/context_forward.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <vector>

namespace Fig
{
    class Object;
    class Function
    {
    public:
        std::size_t id;
        Ast::FunctionParameters paras;
        TypeInfo retType;
        Ast::BlockStatement body;

        bool isBuiltin = false;
        std::function<std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>> &)> builtin;
        int builtinParamCount = -1;

        std::shared_ptr<Context> closureContext;

        // ===== Constructors =====
        Function() :
            id(nextId()) {}

        Function(Ast::FunctionParameters _paras, TypeInfo _retType, Ast::BlockStatement _body, ContextPtr _closureContext) :
            id(nextId()), // 分配唯一 ID
            paras(std::move(_paras)),
            retType(std::move(_retType)),
            body(std::move(_body)),
            closureContext(std::move(_closureContext))
        {
        }

        Function(std::function<std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>> &)> fn, int argc) :
            id(nextId()), isBuiltin(true), builtin(fn), builtinParamCount(argc) {}

        // ===== Copy / Move =====
        Function(const Function &other) = default;
        Function(Function &&) noexcept = default;
        Function &operator=(const Function &) = default;
        Function &operator=(Function &&) noexcept = default;

        // ===== Comparison =====
        bool operator==(const Function &other) const noexcept
        {
            return id == other.id;
        }
        bool operator!=(const Function &other) const noexcept
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
