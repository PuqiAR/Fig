#pragma once

#include <Ast/functionParameters.hpp>
#include <Evaluator/Context/context_forward.hpp>

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
        FString name;

        enum FnType
        {
            Normal,
            Builtin,
            MemberType
        } type;

        union
        {
            struct
            {
                Ast::FunctionParameters paras;
                TypeInfo retType;

                Ast::BlockStatement body;
            };
            std::function<std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>> &)> builtin;
            std::function<std::shared_ptr<Object>(std::shared_ptr<Object>,
                                                  const std::vector<std::shared_ptr<Object>> &)>
                mtFn;
        };

        int builtinParamCount = -1;

        std::shared_ptr<Context> closureContext;

        // ===== Constructors =====
        Function() : id(nextId()), type(Normal)
        {
            // 需要初始化 union ！
            new (&paras) Ast::FunctionParameters();
            new (&retType) TypeInfo();
            new (&body) Ast::BlockStatement();
        }

        Function(const FString &_name,
                 Ast::FunctionParameters _paras,
                 TypeInfo _retType,
                 Ast::BlockStatement _body,
                 ContextPtr _closureContext) :
            id(nextId()), // 分配唯一 ID
            name(_name),
            paras(std::move(_paras)),
            retType(std::move(_retType)),
            body(std::move(_body)),
            closureContext(std::move(_closureContext))
        {
            type = Normal;
        }

        Function(const FString &_name, std::function<std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>> &)> fn, int argc) :
            id(nextId()), name(_name), type(Builtin), builtin(fn), builtinParamCount(argc)
        {
            type = Builtin;
        }

        Function(const FString &_name, std::function<std::shared_ptr<Object>(std::shared_ptr<Object>,
                                                       const std::vector<std::shared_ptr<Object>> &)> fn,
                 int argc) :
            id(nextId()), name(_name), type(MemberType), mtFn(fn), builtinParamCount(argc)
        {
            type = MemberType;
        }

        // ===== Copy / Move =====
        Function(const Function &other) { copyFrom(other); }
        Function &operator=(const Function &other)
        {
            if (this != &other)
            {
                destroy();
                copyFrom(other);
            }
            return *this;
        };

        ~Function() { destroy(); }

        // ===== Comparison =====
        bool operator==(const Function &other) const noexcept { return id == other.id; }
        bool operator!=(const Function &other) const noexcept { return !(*this == other); }

    private:
        static std::size_t nextId()
        {
            static std::atomic<std::size_t> counter{1};
            return counter++;
        }
        void destroy()
        {
            switch (type)
            {
                case Normal:
                    paras.~FunctionParameters();
                    retType.~TypeInfo();
                    body.~shared_ptr();
                    break;
                case Builtin: builtin.~function(); break;
                case MemberType: mtFn.~function(); break;
            }
        }

        void copyFrom(const Function &other)
        {
            name = other.name;
            type = other.type;
            id = nextId(); // 每个复制都生成新的ID
            builtinParamCount = other.builtinParamCount;
            closureContext = other.closureContext;

            switch (type)
            {
                case Normal:
                    new (&paras) Ast::FunctionParameters(other.paras);
                    new (&retType) TypeInfo(other.retType);
                    new (&body) Ast::BlockStatement(other.body);
                    break;
                case Builtin:
                    new (&builtin) std::function<std::shared_ptr<Object>(const std::vector<std::shared_ptr<Object>> &)>(
                        other.builtin);
                    break;
                case MemberType:
                    new (&mtFn) std::function<std::shared_ptr<Object>(
                        std::shared_ptr<Object>, const std::vector<std::shared_ptr<Object>> &)>(other.mtFn);
                    break;
            }
        }
    };
} // namespace Fig
