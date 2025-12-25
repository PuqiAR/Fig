#include <Ast/ast.hpp>

#include <Context/context.hpp>
#include <Error/error.hpp>
#include <Module/builtins.hpp>
#include <Value/LvObject.hpp>


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
            Continue
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

        bool isNormal() const { return flow == Flow::Normal; }
        bool shouldReturn() const { return flow == Flow::Return; }
        bool shouldBreak() const { return flow == Flow::Break; }
        bool shouldContinue() const { return flow == Flow::Continue; }
    };

    class Evaluator
    {
    private:
        ContextPtr global;
    public:

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

        void RegisterBuiltins()
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
                    std::make_shared<Object>(f)
                );
            }

            for (auto &[name, val] : Builtins::builtinValues)
            {
                global->def(
                    name,
                    val->getTypeInfo(),
                    AccessModifier::Const,
                    val
                );
            }
        }
        /* Left-value eval*/
        LvObject evalVarExpr(Ast::VarExpr, ContextPtr);
        LvObject evalMemberExpr(Ast::MemberExpr, ContextPtr); // a.b
        LvObject evalIndexExpr(Ast::IndexExpr, ContextPtr); // a[b]

        LvObject evalLv(Ast::Expression, ContextPtr); // for access: a.b / index a[b]

        /* Right-value eval*/
        
        RvObject evalBinary(Ast::BinaryExpr, ContextPtr); // normal binary expr: +, -, *....
        RvObject evalUnary(Ast::UnaryExpr, ContextPtr); // unary expr
        RvObject evalTernary(Ast::TernaryExpr, ContextPtr); // ternary expr

        RvObject evalFunctionCall(const Function&, const Ast::FunctionArguments&, const FString& ,ContextPtr);    // function call
        RvObject eval(Ast::Expression, ContextPtr);
        
        StatementResult evalBlockStatement(Ast::BlockStatement, ContextPtr); // block
        StatementResult evalStatement(Ast::Statement, ContextPtr);  // statement

        StatementResult Run(std::vector<Ast::AstBase>); // Entry
        
        void printStackTrace();
    };
}; // namespace Fig