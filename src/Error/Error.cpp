/*!
    @file src/Error/Error.cpp
    @brief 错误报告实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <Core/Core.hpp>
#include <Error/Error.hpp>

#include <sstream>

namespace Fig
{
    void ColoredPrint(const char *color, const char *msg, std::ostream &ost = CoreIO::GetStdErr())
    {
        ost << color << msg << TerminalColors::Reset;
    }

    void ColoredPrint(const char *color, const std::string &msg, std::ostream &ost = CoreIO::GetStdErr())
    {
        ost << color << msg << TerminalColors::Reset;
    }

    void ColoredPrint(const char *color, const String &msg, std::ostream &ost = CoreIO::GetStdErr())
    {
        ost << color << msg << TerminalColors::Reset;
    }

    std::string MultipleStr(const char *c, size_t n)
    {
        std::string buf;
        for (size_t i = 0; i < n; ++i) { buf += c; }
        return buf;
    }

    const char *ErrorTypeToString(ErrorType type)
    {
        using enum ErrorType;
        switch (type)
        {
            case UnusedSymbol: return "UnusedSymbol";

            case MayBeNull: return "MaybeNull";

            case UnterminatedString: return "UnterminatedString";
            case UnterminatedComments: return "UnterminatedComments";
            case InvalidNumberLiteral: return "InvalidNumberLiteral";
            case InvalidCharacter: return "InvalidCharacter";
            case InvalidSymbol: return "InvalidSymbol";

            case ExpectedExpression: return "ExpectedExpression";
            case SyntaxError: return "SyntaxError";

            // default: return "Some one forgot to add case to `ErrorTypeToString`";
        }
    }

    void PrintSystemInfos()
    {
        std::ostream &err = CoreIO::GetStdErr();
        std::stringstream build_info;
        build_info << "\r🌘 Fig v" << Core::VERSION << " on " << Core::PLATFORM << ' ' << Core::ARCH << '['
                   << Core::COMPILER << ']' << '\n'
                   << "   Build Time: " << Core::COMPILE_TIME;

        const std::string &build_info_str = build_info.str();
        err << MultipleStr("─", build_info_str.size()) << '\n';
        err << build_info_str << '\n';
        err << MultipleStr("─", build_info_str.size()) << '\n';
    }

    void PrintErrorInfo(const Error &error, const SourceManager &srcManager)
    {
        static constexpr const char *MinorColor = "\033[38;2;138;227;198m";
        static constexpr const char *MediumColor = "\033[38;2;255;199;95m";
        static constexpr const char *CriticalColor = "\033[38;2;255;107;107m";

        namespace TC = TerminalColors;
        std::ostream &err = CoreIO::GetStdErr();

        uint8_t level = ErrorLevel(error.type);
        // const char *level_name = (level == 1 ? "Minor" : (level == 2 ? "Medium" : "Critical"));
        const char *level_color = (level == 1 ? MinorColor : (level == 2 ? MediumColor : CriticalColor));

        err << "🔥 "
            << level_color
            //<< '(' << level_name << ')'
            << 'E' << static_cast<int>(error.type) << TC::Reset << ": " << level_color << ErrorTypeToString(error.type)
            << TC::Reset << '\n';

        const SourceLocation &location = error.location;

        err << TC::DarkGray << "  ┌─> Fn " << TC::Cyan << '\'' << location.packageName << '.' << location.functionName
            << '\'' << " " << location.fileName << " (" << TC::DarkGray << location.sp.line << ":" << location.sp.column
            << TC::Cyan << ')' << TC::Reset << '\n';
        err << TC::DarkGray << "  │" << '\n' << "  │" << TC::Reset << '\n';

        // 尝试打印上3行 下2行

        int64_t line_start = location.sp.line - 3, line_end = location.sp.line + 2;
        while (!srcManager.HasLine(line_end)) { --line_end; }
        while (!srcManager.HasLine(line_start)) { ++line_start; }

        const auto &getLineNumWidth = [](size_t l) {
            unsigned int cnt = 0;
            while (l != 0)
            {
                l = l / 10;
                cnt++;
            }
            return cnt;
        };
        unsigned int max_line_number_width = getLineNumWidth(line_end);
        for (size_t i = line_start; i <= line_end; ++i)
        {
            unsigned int offset = 2 + 2 + 1;
            //                     '  └─ '
            if (i == location.sp.line) { err << TC::DarkGray << "  └─ " << TC::Reset; }
            else if (i < location.sp.line) { err << TC::DarkGray << "  │  " << TC::Reset; }
            else
            {
                err << MultipleStr(" ", offset);
            }
            unsigned int cur_line_number_width = getLineNumWidth(i);

            err << MultipleStr(" ", max_line_number_width - cur_line_number_width) << TC::Yellow << i << TC::Reset;
            err << " │ " << srcManager.GetLine(i) << '\n';
            if (i == location.sp.line)
            {
                unsigned int error_col_offset = offset + 1 + max_line_number_width + 2;
                err << MultipleStr(" ", error_col_offset) << MultipleStr(" ", location.sp.column - 1) << TC::LightGreen
                    << MultipleStr("^", location.sp.tok_length) << TC::Reset << '\n';

                err << MultipleStr(" ", error_col_offset)
                    << MultipleStr(" ", location.sp.column - 1 + location.sp.tok_length / 2) << "╰─ " << level_color
                    << error.message << TC::Reset << "\n\n";
            }
        }
        err << "\n";
        err << "❓ " << TC::DarkGray << "Thrower: " << error.thrower_loc.function_name() << " ("
            << error.thrower_loc.file_name() << ":" << error.thrower_loc.line() << ")" << TC::Reset << "\n";
        err << "💡 " << TC::Blue << "Suggestion: " << error.suggestion << TC::Reset;
    }

    void ReportError(const Error &error, const SourceManager &srcManager)
    {
        assert(srcManager.read && "ReportError: srcManager doesn't read source");
        assert(srcManager.HasLine(error.location.sp.line));

        PrintSystemInfos();
        PrintErrorInfo(error, srcManager);
    }

    void ReportErrors(const std::vector<Error> &errors, const SourceManager &srcManager)
    {
        std::ostream &ost = CoreIO::GetStdErr();
        PrintSystemInfos();
        for (const auto &err : errors)
        {
            PrintErrorInfo(err, srcManager);
            ost << '\n';
        }
    }
}; // namespace Fig