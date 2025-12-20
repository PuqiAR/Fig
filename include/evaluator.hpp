#pragma once

#include <builtins.hpp>
#include <error.hpp>
#include <fig_string.hpp>
#include <ast.hpp>
#include <value.hpp>
#include <context.hpp>
#include <parser.hpp>

namespace Fig
{

    template <const char *errName>
    class EvaluatorError final : public AddressableError
    {
    public:
        virtual FString toString() const override
        {
            std::string msg = std::format("[Eve: {}] {} in [{}] {}", errName, std::string(this->message.begin(), this->message.end()), this->src_loc.file_name(), this->src_loc.function_name());
            return FString(msg);
        }
        using AddressableError::AddressableError;
        explicit EvaluatorError(FStringView _msg,
                                Ast::AstAddressInfo aai,
                                std::source_location loc = std::source_location::current()) :
            AddressableError(_msg, aai.line, aai.column, loc)
        {
        }
    };
    struct StatementResult
    {
        Value result;
        enum class Flow
        {
            Normal,
            Return,
            Break,
            Continue
        } flow;

        StatementResult(Value val, Flow f = Flow::Normal) :
            result(val), flow(f)
        {
        }

        static StatementResult normal(Value val = Value::getNullInstance())
        {
            return StatementResult(val, Flow::Normal);
        }
        static StatementResult returnFlow(Value val)
        {
            return StatementResult(val, Flow::Return);
        }
        static StatementResult breakFlow()
        {
            return StatementResult(Value::getNullInstance(), Flow::Break);
        }
        static StatementResult continueFlow()
        {
            return StatementResult(Value::getNullInstance(), Flow::Continue);
        }

        bool isNormal() const { return flow == Flow::Normal; }
        bool shouldReturn() const { return flow == Flow::Return; }
        bool shouldBreak() const { return flow == Flow::Break; }
        bool shouldContinue() const { return flow == Flow::Continue; }
    };

    class Evaluator
    {
    private:
        std::vector<Ast::AstBase> asts;
        std::shared_ptr<Context> globalContext;
        std::shared_ptr<Context> currentContext;

        Ast::AstAddressInfo currentAddressInfo;

    public:
        Evaluator(const std::vector<Ast::AstBase> &a) :
            asts(a)
        {
            globalContext = std::make_shared<Context>(FString(u8"global"));
            currentContext = globalContext;

            for (auto &[name, fn] : Builtins::builtinFunctions)
            {
                int argc = Builtins::getBuiltinFunctionParamCount(name);
                Function f(fn, argc);
                globalContext->def(
                    name,
                    ValueType::Function,
                    AccessModifier::PublicConst,
                    Value(f));
            }

            for (auto &[name, val] : Builtins::builtinValues)
            {
                globalContext->def(
                    name,
                    val.getTypeInfo(),
                    AccessModifier::PublicConst,
                    val);
            }
        }

        std::shared_ptr<Context> getCurrentContext() { return currentContext; }
        std::shared_ptr<Context> getGlobalContext() { return globalContext; }

        Value __evalOp(Ast::Operator, const Value &, const Value & = Value::getNullInstance());
        Value evalBinary(const Ast::BinaryExpr &);
        Value evalUnary(const Ast::UnaryExpr &);

        StatementResult evalStatement(const Ast::Statement &);

        Value evalFunctionCall(const Function &, const Ast::FunctionArguments &, FString fnName = u8"<anonymous>");

        Value eval(Ast::Expression);
        void run();
        void printStackTrace() const;
    };

} // namespace Fig
