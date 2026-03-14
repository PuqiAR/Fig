/*!
    @file src/Repl/Repl.hpp
    @brief Repl定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-11
*/

#pragma once

#include <Core/Core.hpp>
#include <Core/SourceLocations.hpp>
#include <SourceManager/SourceManager.hpp>

#include <Compiler/Compiler.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Sema/Analyzer.hpp>
#include <VM/VM.hpp>

#include <Utils/ConsoleSize.hpp>

namespace Fig
{
    class Repl
    {
    private:
        size_t rline = 1;

        std::istream &in;
        std::ostream &out;
        std::ostream &err;

    public:
        Repl(std::istream &_in, std::ostream &_out, std::ostream &_err) :
            in(_in), out(_out), err(_err)
        {
        }

        void PrintInfo()
        {
            out << std::format("Fig {}, copyright (c) 2025-2026 PuqiAR, under the {} License\n",
                Core::VERSION,
                Core::LICENSE);
            out << std::format("Build time: {} [{} x{} on {}]\n",
                Core::COMPILE_TIME,
                Core::COMPILER,
                Core::ARCH,
                Core::PLATFORM);
            out << "Type '#exit' to exit, '#clear' to clear the the screen, '#license' to see the full license, '#logo' to see a GREAT logo\n";
        }

        void ClearConsole()
        {
            // \033[2J: 清除整个屏幕
            // \033[H: 将光标移动到左上角
            out << "\033[2J\033[H" << std::flush;
        }

        void PrintLicense()
        {
            static DynArray<std::string> license_lines{};
            if (license_lines.empty())
            {
                std::string l;
                bool        last_was_r = false;

                for (size_t i = 0; i < Core::LICENSE_TEXT.size(); ++i)
                {
                    char c = Core::LICENSE_TEXT[i];

                    if (c == '\r')
                    {
                        last_was_r = true;
                        continue;
                    }

                    if (c == '\n')
                    {
                        if (!l.empty() || last_was_r)
                        {
                            license_lines.push_back(l);
                            l.clear();
                        }
                        last_was_r = false;
                        continue;
                    }

                    if (last_was_r)
                    {
                        if (!l.empty())
                        {
                            license_lines.push_back(l);
                            l.clear();
                        }
                        last_was_r = false;
                    }

                    l += c;
                }

                if (!l.empty() || last_was_r)
                {
                    license_lines.push_back(l);
                }
            }

            unsigned int lines_per_page = 50;
            unsigned int page_printed   = 0;
            unsigned int lines_printed  = 0;
            unsigned int total_lines    = license_lines.size();

            while (true)
            {
                auto _csize = Utils::getConsoleSize();
                if (_csize)
                {
                    lines_per_page = static_cast<unsigned int>((*_csize).first * 0.75);
                    if (lines_per_page < 1)
                        lines_per_page = 1;
                }

                unsigned int lines_this_page =
                    std::min(lines_per_page, total_lines - lines_printed);

                for (unsigned int i = 0; i < lines_this_page; ++i)
                {
                    out << license_lines[lines_printed + i] << '\n';
                }

                lines_printed += lines_this_page;
                page_printed++;

                if (lines_printed >= total_lines)
                {
                    break;
                }

                unsigned int pages_total = (total_lines + lines_per_page - 1) / lines_per_page;

                out << "\n"
                    << std::format(
                           "Press any key to continue... ({}/{})", page_printed, pages_total);

                in.get();
                out << '\n';
                ++rline;
            }
        }
        void PrintError(const Error &error, const String &source)
        {
            err << "Oops! An error occurred!";
            err << "🔥 " << 'E' << static_cast<int>(error.type) << ": " << error.message << '\n';
            err << "Line " << rline << ", `" << source << "`\n";
            err << "Suggestion: " << error.suggestion << '\n';
            err << std::format("Thrower: {} ({}:{}:{})\n",
                error.thrower_loc.function_name(),
                error.thrower_loc.file_name(),
                error.thrower_loc.line(),
                error.thrower_loc.column());
        }

        unsigned int Start() // exit code: unsigned int
        {
            // TODO:  多行输入 Repl

            PrintInfo();

            String fileName = "[REPL]";
            String filePath = "src/Repl/Repl.hpp";

            SourceManager manager;

            VM vm;

            Value v = Value::GetNullInstance();

            while (true)
            {
                std::string buf;

                out << ">";

                std::getline(in, buf);

                if (buf.empty())
                {
                    continue;
                }
                else if (buf == "#exit")
                {
                    return 0;
                }
                else if (buf == "#clear")
                {
                    ClearConsole();
                    continue;
                }
                else if (buf == "#license")
                {
                    PrintLicense();
                    continue;
                }
                else if (buf == "#logo")
                {
                    out << Core::LOGO << '\n';
                    continue;
                }

                String source(buf);

                Lexer  lexer(buf, fileName);
                Parser parser(lexer, manager, fileName);

                auto _program = parser.Parse();
                if (!_program)
                {
                    PrintError(_program.error(), source);
                    continue;
                }

                Program *program = *_program;

                Analyzer analyzer(manager);
                auto     result = analyzer.Analyze(program);

                if (!result)
                {
                    PrintError(result.error(), source);
                    continue;
                }

                Compiler compiler(manager, analyzer.GetDiagnostics());

                auto compile_result = compiler.Compile(program);
                if (!compile_result)
                {
                    PrintError(compile_result.error(), source);
                    continue;
                }

                CompiledModule *compiledModule = *compile_result;

                auto exe_result = vm.Execute(compiledModule);
                if (!exe_result)
                {
                    PrintError(exe_result.error(), source);
                    continue;
                }

                v = *exe_result;
                if (!v.IsNull())
                {
                    out << v.ToString() << '\n';
                }
            }

            return (v.IsInt() ? v.AsInt() : 0);
        }
    };
}; // namespace Fig