/*!
    @file src/VM/Entry.hpp
    @brief vm入口实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-13
*/


#include <VM/Entry.hpp>

#include <filesystem>

#include <Core/Core.hpp>
#include <SourceManager/SourceManager.hpp>

#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Sema/Analyzer.hpp>
#include <Compiler/Compiler.hpp>
#include <VM/VM.hpp>

namespace Fig::Entry
{
    void RunFromPath(const String &path)
    {
        namespace fs = std::filesystem;

        fs::path _fspath(path.toStdString());

        if (!fs::exists(_fspath))
        {
            CoreIO::GetStdErr() << "File not found: " << path << '\n';
            std::exit(1);
        }

        if (!_fspath.has_extension() || _fspath.extension() != ".fig")
        {
            CoreIO::GetStdErr() << "Not a valid Fig-lang source code\n";
            std::exit(1);
        }

        String fileName(_fspath.filename().string());

        SourceManager manager(path);
        manager.Read();

        if (!manager.read)
        {
            CoreIO::GetStdErr() << "Could not read file: " << path << '\n';
            std::exit(1);
        }
        
        const String &source = manager.GetSource();

        Lexer lexer(source, fileName);
        Parser parser(lexer, manager, fileName);

        auto parse_result = parser.Parse();
        if (!parse_result)
        {
            ReportError(parse_result.error(), manager);
            std::exit(1);
        }
        Program *program = *parse_result;

        Analyzer analyer(manager);
        auto analyze_result = analyer.Analyze(program);

        if (!analyze_result)
        {
            ReportError(analyze_result.error(), manager);
            std::exit(1);
        }

        Diagnostics diagnostics;
        Compiler compiler(manager, diagnostics);

        auto compile_result = compiler.Compile(program);
        diagnostics.EmitAll(manager);

        if (!compile_result)
        {
            ReportError(compile_result.error(), manager);
            std::exit(1);
        }

        CompiledModule *compiledModule = *compile_result;

        VM vm;

        auto execute_result = vm.Execute(compiledModule);
        if (!execute_result)
        {
            ReportError(execute_result.error(), manager);
            std::exit(1);
        }

        delete compiledModule;
    }
}; // namespace Fig::Entry