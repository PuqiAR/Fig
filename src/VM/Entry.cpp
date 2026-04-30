/*!
    @file src/VM/Entry.hpp
    @brief vm入口实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-13
*/

#include <VM/Entry.hpp>

#include <chrono>
#include <filesystem>

#include <Core/Core.hpp>
#include <SourceManager/SourceManager.hpp>

#include <Bytecode/Disassembler.hpp>
#include <Compiler/Compiler.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Repl/Repl.hpp>
#include <Sema/Analyzer.hpp>
#include <VM/VM.hpp>

namespace Fig::Entry
{
    void RunFromPath(const String &path, const Config &conf)
    {
        namespace fs = std::filesystem;

        using clock = std::chrono::steady_clock;

        auto format_print_time = [](std::chrono::nanoseconds nsecs) {
            auto &out   = CoreIO::GetStdOut();
            auto  count = nsecs.count();

            auto old_flags     = out.flags();
            auto old_precision = out.precision();

            if (count < 1'000)
            {
                // < 1μs 纳秒
                out << count << "ns";
            }
            else if (count < 1'000'000)
            {
                // 1μs ~ 1ms 微秒 保留 2 位小数
                out << std::fixed << std::setprecision(2) << (count / 1'000.0) << "μs";
            }
            else if (count < 1'000'000'000)
            {
                // 1ms ~ 1s 毫秒 保留 2 位小数
                out << std::fixed << std::setprecision(2) << (count / 1'000'000.0) << "ms";
            }
            else
            {
                // >= 1s 秒 保留 3 位小数
                out << std::fixed << std::setprecision(3) << (count / 1'000'000'000.0) << "s";
            }

            out.flags(old_flags);
            out.precision(old_precision);
        };

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

        Diagnostics diagnostics;

        Parser parser(lexer, manager, fileName, diagnostics);

        auto parse_start  = clock::now();
        auto parse_result = parser.Parse();
        auto parse_end    = clock::now();

        if (!parse_result)
        {
            ReportError(parse_result.error(), manager);
            std::exit(1);
        }

        Program *program = *parse_result;

        Analyzer analyer(manager);

        auto analyze_start  = clock::now();
        auto analyze_result = analyer.Analyze(program);
        auto analyze_end    = clock::now();

        if (!analyze_result)
        {
            ReportError(analyze_result.error(), manager);
            std::exit(1);
        }

        Compiler compiler(manager, diagnostics);

        auto compile_start  = clock::now();
        auto compile_result = compiler.Compile(program);
        auto compile_end    = clock::now();

        diagnostics.EmitAll(manager);

        if (!compile_result)
        {
            ReportError(compile_result.error(), manager);
            std::exit(1);
        }

        CompiledModule *compiledModule = *compile_result;

        if (conf.dump)
        {
            Disassembler disassembler;
            disassembler.DisassembleModule(compiledModule);
        }

        VM vm;

        auto execute_start  = clock::now();
        auto execute_result = vm.Execute(compiledModule);
        auto execute_end    = clock::now();

        if (!execute_result)
        {
            ReportError(execute_result.error(), manager);
            std::exit(1);
        }

        if (conf.pregs)
        {
            vm.PrintRegisters();
        }

        if (conf.time)
        {
            auto parse_time = parse_end - parse_start;
            CoreIO::GetStdOut() << "Parse: ";
            format_print_time(parse_time);
            CoreIO::GetStdOut() << " | ";

            auto analyze_time = analyze_end - analyze_start;
            CoreIO::GetStdOut() << "Analyze: ";
            format_print_time(analyze_time);
            CoreIO::GetStdOut() << " | ";

            auto compile_time = compile_end - compile_start;
            CoreIO::GetStdOut() << "Compile: ";
            format_print_time(compile_time);
            CoreIO::GetStdOut() << " | ";

            auto execute_time = execute_end - execute_start;
            CoreIO::GetStdOut() << "Execute: ";
            format_print_time(execute_time);
            CoreIO::GetStdOut() << " | ";

            auto total = parse_time + analyze_time + compile_time + execute_time;
            CoreIO::GetStdOut() << "Total: ";
            format_print_time(total);
            CoreIO::GetStdOut() << '\n';
        }

        delete compiledModule;
    }

    std::uint32_t RunRepl()
    {
        Repl          repl(CoreIO::GetStdCin(), CoreIO::GetStdOut(), CoreIO::GetStdErr());
        std::uint32_t result = repl.Start();

        CoreIO::GetStdOut() << "Repl exited with code " << result << '\n';
        return result;
    }
}; // namespace Fig::Entry