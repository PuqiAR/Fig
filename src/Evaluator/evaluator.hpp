#include "Ast/Expressions/InitExpr.hpp"
#include <Ast/Statements/ImplementSt.hpp>
#include <Ast/Statements/InterfaceDefSt.hpp>
#include <Value/Type.hpp>
#include <Ast/ast.hpp>

#include <Context/context.hpp>
#include <Error/error.hpp>
#include <Module/builtins.hpp>
#include <Value/LvObject.hpp>
#include <filesystem>

namespace Fig
{
    struct StatementResult
    {
        ObjectPtr result;
        enum class Flow
        {
            Normal,
            Return,
            Break,
            Continue,
            Error
        } flow;

        StatementResult(ObjectPtr val, Flow f = Flow::Normal) :
            result(val), flow(f)
        {
        }

        static StatementResult normal(ObjectPtr val = Object::getNullInstance())
        {
            return StatementResult(val, Flow::Normal);
        }
        static StatementResult returnFlow(ObjectPtr val)
        {
            return StatementResult(val, Flow::Return);
        }
        static StatementResult breakFlow()
        {
            return StatementResult(Object::getNullInstance(), Flow::Break);
        }
        static StatementResult continueFlow()
        {
            return StatementResult(Object::getNullInstance(), Flow::Continue);
        }
        static StatementResult errorFlow(ObjectPtr val)
        {
            return StatementResult(val, Flow::Error);
        }

        bool isNormal() const { return flow == Flow::Normal; }
        bool shouldReturn() const { return flow == Flow::Return; }
        bool shouldBreak() const { return flow == Flow::Break; }
        bool shouldContinue() const { return flow == Flow::Continue; }
        bool isError() const { return flow == Flow::Error; }
    };

    class Evaluator
    {
    private:
        ContextPtr global;

    public:
        FString sourcePath;
        std::vector<FString> sourceLines;

        void SetSourcePath(const FString &sp)
        {
            sourcePath = sp;
        }

        void SetSourceLines(const std::vector<FString> &sl)
        {
            sourceLines = sl;
        }

        void SetGlobalContext(ContextPtr ctx)
        {
            assert(ctx != nullptr);
            global = ctx;
        }

        void CreateGlobalContext()
        {
            global = std::make_shared<Context>(
                FString(u8"<Global>"));
        }

        void RegisterBuiltins() // only function
        {
            assert(global != nullptr);

            for (auto &[name, fn] : Builtins::builtinFunctions)
            {
                int argc = Builtins::getBuiltinFunctionParamCount(name);
                Function f(fn, argc);
                global->def(
                    name,
                    ValueType::Function,
                    AccessModifier::Const,
                    std::make_shared<Object>(f));
            }
        }

        void RegisterBuiltinsValue()
        {
            assert(global != nullptr);

            for (auto &[name, val] : Builtins::builtinValues)
            {
                global->def(
                    name,
                    val->getTypeInfo(),
                    AccessModifier::Const,
                    val);
            }
        }

        bool isInterfaceSignatureMatch(const Ast::ImplementMethod &, const Ast::InterfaceMethod &);

        /* Left-value eval*/
        LvObject evalVarExpr(Ast::VarExpr, ContextPtr);
        LvObject evalMemberExpr(Ast::MemberExpr, ContextPtr); // a.b
        LvObject evalIndexExpr(Ast::IndexExpr, ContextPtr);   // a[b]

        LvObject evalLv(Ast::Expression, ContextPtr); // for access: a.b / index a[b]

        /* Right-value eval*/
        RvObject evalInitExpr(Ast::InitExpr, ContextPtr); // only allows evalUnary to call
        RvObject evalBinary(Ast::BinaryExpr, ContextPtr);   // normal binary expr: +, -, *....
        RvObject evalUnary(Ast::UnaryExpr, ContextPtr);     // unary expr
        RvObject evalTernary(Ast::TernaryExpr, ContextPtr); // ternary expr

        RvObject evalFunctionCall(const Function &, const Ast::FunctionArguments &, const FString &, ContextPtr); // function call
        RvObject eval(Ast::Expression, ContextPtr);

        StatementResult evalBlockStatement(Ast::BlockStatement, ContextPtr); // block
        StatementResult evalStatement(Ast::Statement, ContextPtr);           // statement

        std::filesystem::path resolveModulePath(const std::vector<FString> &);
        ContextPtr loadModule(const std::filesystem::path &);

        StatementResult evalImportSt(Ast::Import, ContextPtr);

        StatementResult Run(std::vector<Ast::AstBase>); // Entry

        void printStackTrace();
    };
}; // namespace Fig