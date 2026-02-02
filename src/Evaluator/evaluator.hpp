#pragma once
#include <Ast/Expressions/FunctionCall.hpp>
#include <Ast/Expressions/InitExpr.hpp>
#include <Ast/Statements/ImplementSt.hpp>
#include <Ast/Statements/InterfaceDefSt.hpp>
#include <Evaluator/Value/Type.hpp>
#include <Ast/ast.hpp>

#include <Evaluator/Context/context.hpp>
#include <Error/error.hpp>
#include <Module/builtins.hpp>
#include <Evaluator/Value/LvObject.hpp>
#include <filesystem>

#include <Evaluator/Core/StatementResult.hpp>

namespace Fig
{
    class Evaluator
    {
    private:
        ContextPtr global;

    public:
        FString sourcePath;
        std::vector<FString> sourceLines;

        void SetSourcePath(const FString &sp) { sourcePath = sp; }

        void SetSourceLines(const std::vector<FString> &sl) { sourceLines = sl; }

        void SetGlobalContext(ContextPtr ctx)
        {
            assert(ctx != nullptr);
            global = ctx;
        }

        void CreateGlobalContext() { global = std::make_shared<Context>(FString(u8"<Global>")); }

        void RegisterBuiltins() // only function
        {
            assert(global != nullptr);

            for (auto &[name, fn] : Builtins::getBuiltinFunctions())
            {
                int argc = Builtins::getBuiltinFunctionParamCount(name);
                Function f(name, fn, argc);
                global->def(name, ValueType::Function, AccessModifier::Const, std::make_shared<Object>(f));
            }
        }

        void RegisterBuiltinsValue()
        {
            assert(global != nullptr);

            for (auto &[name, val] : Builtins::getBuiltinValues())
            {
                global->def(name, val->getTypeInfo(), AccessModifier::Const, val);
            }
        }

        bool isInterfaceSignatureMatch(const Ast::ImplementMethod &, const Ast::InterfaceMethod &);

        /* Left-value eval*/
        LvObject evalVarExpr(Ast::VarExpr, ContextPtr);
        LvObject evalMemberExpr(Ast::MemberExpr, ContextPtr); // a.b
        LvObject evalIndexExpr(Ast::IndexExpr, ContextPtr);   // a[b]

        LvObject evalLv(Ast::Expression, ContextPtr); // for access: a.b / index a[b]

        /* Right-value eval*/
        RvObject evalInitExpr(Ast::InitExpr, ContextPtr);   // only allows evalUnary to call
        RvObject evalBinary(Ast::BinaryExpr, ContextPtr);   // normal binary expr: +, -, *....
        RvObject evalUnary(Ast::UnaryExpr, ContextPtr);     // unary expr
        RvObject evalTernary(Ast::TernaryExpr, ContextPtr); // ternary expr

        RvObject executeFunction(const Function &fn, const Ast::FunctionCallArgs &, ContextPtr); // fn, fn context

        RvObject evalFunctionCall(const Ast::FunctionCall &,
                                  ContextPtr); // function call

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