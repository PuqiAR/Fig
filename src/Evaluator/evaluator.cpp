#include <Utils/utils.hpp>
#include <Parser/parser.hpp>

#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

#include <filesystem>
#include <fstream>
#include <memory>

#ifndef SourceInfo
    #define SourceInfo(ptr) (ptr->sourcePath), (ptr->sourceLines)
#endif

namespace Fig
{

    bool Evaluator::isInterfaceSignatureMatch(const Ast::ImplementMethod &implMethod,
                                              const Ast::InterfaceMethod &ifMethod)
    {
        return implMethod.name == ifMethod.name && implMethod.paras == ifMethod.paras;
    }

    StatementResult Evaluator::evalBlockStatement(Ast::BlockStatement block, ContextPtr ctx)
    {
        StatementResult sr = StatementResult::normal();
        for (const Ast::Statement &stmt : block->stmts)
        {
            sr = evalStatement(stmt, ctx);
            if (!sr.isNormal()) { return sr; }
        }
        return sr;
    }

    ContextPtr Evaluator::loadModule(const std::filesystem::path &path)
    {
        FString modSourcePath(path.string());
        std::ifstream file(path);
        assert(file.is_open());

        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        std::vector<FString> modSourceLines = Utils::splitSource(FString(source));

        Lexer lexer((FString(source)), modSourcePath, modSourceLines);
        Parser parser(lexer, modSourcePath, modSourceLines);
        std::vector<Ast::AstBase> asts;

        asts = parser.parseAll();

        Evaluator evaluator;
        evaluator.SetSourcePath(modSourcePath);
        evaluator.SetSourceLines(modSourceLines);

        ContextPtr modctx = std::make_shared<Context>(FString(std::format("<Module at {}>", path.string())), nullptr);

        evaluator.SetGlobalContext(modctx);
        evaluator.RegisterBuiltinsValue();
        evaluator.Run(asts); // error upward pass-by, log outside, we have already keep info in evaluator error

        return evaluator.global;
    }

    StatementResult Evaluator::evalImportSt(Ast::Import i, ContextPtr ctx)
    {
        const std::vector<FString> &pathVec = i->path;

        const FString &modName = pathVec.at(pathVec.size() - 1); // pathVec at least has 1 element
        if (modName == u8"_builtins")
        {
            RegisterBuiltins();
            return StatementResult::normal();
        }
        auto path = resolveModulePath(pathVec);
        ContextPtr modCtx = loadModule(path);

        ctx->getImplRegistry().insert(modCtx->getImplRegistry().begin(), modCtx->getImplRegistry().end()); // load impl

        // std::cerr << modName.toBasicString() << '\n'; DEBUG

        if (ctx->containsInThisScope(modName))
        {
            throw EvaluatorError(
                u8"RedeclarationError", std::format("{} has already been declared.", modName.toBasicString()), i);
        }
        ctx->def(
            modName, ValueType::Module, AccessModifier::PublicConst, std::make_shared<Object>(Module(modName, modCtx)));
        return StatementResult::normal();
    }

    StatementResult Evaluator::Run(std::vector<Ast::AstBase> asts)
    {
        using Ast::AstType;
        StatementResult sr = StatementResult::normal();
        for (auto &ast : asts)
        {
            Ast::Expression exp;
            if ((exp = std::dynamic_pointer_cast<Ast::ExpressionAst>(ast))) // 保持 dynamic_pointer_cast !
            {
                sr = StatementResult::normal(eval(exp, global));
            }
            else
            {
                // statement
                Ast::Statement stmt = std::static_pointer_cast<Ast::StatementAst>(ast);
                assert(stmt != nullptr);
                sr = evalStatement(stmt, global);
                if (sr.isError())
                {
                    throw EvaluatorError(u8"UncaughtExceptionError",
                                         std::format("Uncaught exception: {}", sr.result->toString().toBasicString()),
                                         stmt);
                }
                if (!sr.isNormal()) { return sr; }
            }
        }

        return sr;
    }

    void Evaluator::printStackTrace()
    {
        if (global) global->printStackTrace();
    }
}; // namespace Fig