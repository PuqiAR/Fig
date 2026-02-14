/*!
    @file src/Error/Error.hpp
    @brief Error定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-13
*/

#pragma once

#include <Core/SourceLocations.hpp>
#include <Deps/Deps.hpp>
#include <SourceManager/SourceManager.hpp>


#include <source_location>

namespace Fig
{
    /*
        0-1000     Minor
        1001-2000  Medium
        2001-3000  Critical
    */
    enum class ErrorType : unsigned int
    {
        /* Minor */
        UnusedSymbol = 0,

        /* Medium */
        MayBeNull = 1001,

        /* Critical */

        // lexer errors
        UnterminatedString = 2001,
        UnterminatedComments,
        InvalidNumberLiteral,
        InvalidCharacter,
        InvalidSymbol,

        // parser errors
        ExpectedExpression,
        SyntaxError,
    };

    const char *ErrorTypeToString(ErrorType type);

    struct Error
    {
        ErrorType type;
        String    message;
        String    suggestion;

        SourceLocation       location;
        std::source_location thrower_loc;

        Error() {}
        Error(ErrorType                 _type,
            const String               &_message,
            const String               &_suggestion,
            const SourceLocation       &_location,
            const std::source_location &_throwerloc = std::source_location::current())
        {
            type        = _type;
            message     = _message;
            suggestion  = _suggestion;
            location    = _location;
            thrower_loc = _throwerloc;
        }
    };

    namespace TerminalColors
    {
        constexpr const char *Reset     = "\033[0m";
        constexpr const char *Bold      = "\033[1m";
        constexpr const char *Dim       = "\033[2m";
        constexpr const char *Italic    = "\033[3m";
        constexpr const char *Underline = "\033[4m";
        constexpr const char *Blink     = "\033[5m";
        constexpr const char *Reverse   = "\033[7m"; // 前背景反色
        constexpr const char *Hidden    = "\033[8m"; // 隐藏文本
        constexpr const char *Strike    = "\033[9m"; // 删除线

        constexpr const char *Black   = "\033[30m";
        constexpr const char *Red     = "\033[31m";
        constexpr const char *Green   = "\033[32m";
        constexpr const char *Yellow  = "\033[33m";
        constexpr const char *Blue    = "\033[34m";
        constexpr const char *Magenta = "\033[35m";
        constexpr const char *Cyan    = "\033[36m";
        constexpr const char *White   = "\033[37m";

        constexpr const char *LightBlack   = "\033[90m";
        constexpr const char *LightRed     = "\033[91m";
        constexpr const char *LightGreen   = "\033[92m";
        constexpr const char *LightYellow  = "\033[93m";
        constexpr const char *LightBlue    = "\033[94m";
        constexpr const char *LightMagenta = "\033[95m";
        constexpr const char *LightCyan    = "\033[96m";
        constexpr const char *LightWhite   = "\033[97m";

        constexpr const char *DarkRed     = "\033[38;2;128;0;0m";
        constexpr const char *DarkGreen   = "\033[38;2;0;100;0m";
        constexpr const char *DarkYellow  = "\033[38;2;128;128;0m";
        constexpr const char *DarkBlue    = "\033[38;2;0;0;128m";
        constexpr const char *DarkMagenta = "\033[38;2;100;0;100m";
        constexpr const char *DarkCyan    = "\033[38;2;0;128;128m";
        constexpr const char *DarkGray    = "\033[38;2;64;64;64m";
        constexpr const char *Gray        = "\033[38;2;128;128;128m";
        constexpr const char *Silver      = "\033[38;2;192;192;192m";

        constexpr const char *Navy        = "\033[38;2;0;0;128m";
        constexpr const char *RoyalBlue   = "\033[38;2;65;105;225m";
        constexpr const char *ForestGreen = "\033[38;2;34;139;34m";
        constexpr const char *Olive       = "\033[38;2;128;128;0m";
        constexpr const char *Teal        = "\033[38;2;0;128;128m";
        constexpr const char *Maroon      = "\033[38;2;128;0;0m";
        constexpr const char *Purple      = "\033[38;2;128;0;128m";
        constexpr const char *Orange      = "\033[38;2;255;165;0m";
        constexpr const char *Gold        = "\033[38;2;255;215;0m";
        constexpr const char *Pink        = "\033[38;2;255;192;203m";
        constexpr const char *Crimson     = "\033[38;2;220;20;60m";

        constexpr const char *OnBlack   = "\033[40m";
        constexpr const char *OnRed     = "\033[41m";
        constexpr const char *OnGreen   = "\033[42m";
        constexpr const char *OnYellow  = "\033[43m";
        constexpr const char *OnBlue    = "\033[44m";
        constexpr const char *OnMagenta = "\033[45m";
        constexpr const char *OnCyan    = "\033[46m";
        constexpr const char *OnWhite   = "\033[47m";

        constexpr const char *OnLightBlack   = "\033[100m";
        constexpr const char *OnLightRed     = "\033[101m";
        constexpr const char *OnLightGreen   = "\033[102m";
        constexpr const char *OnLightYellow  = "\033[103m";
        constexpr const char *OnLightBlue    = "\033[104m";
        constexpr const char *OnLightMagenta = "\033[105m";
        constexpr const char *OnLightCyan    = "\033[106m";
        constexpr const char *OnLightWhite   = "\033[107m";

        constexpr const char *OnDarkBlue    = "\033[48;2;0;0;128m";
        constexpr const char *OnGreenYellow = "\033[48;2;173;255;47m";
        constexpr const char *OnOrange      = "\033[48;2;255;165;0m";
        constexpr const char *OnGray        = "\033[48;2;128;128;128m";
    }; // namespace TerminalColors

    inline uint8_t ErrorLevel(ErrorType t)
    {
        unsigned int id = static_cast<int>(t);
        if (id <= 1000)
        {
            return 1;
        }
        if (id > 1000 && id <= 2000)
        {
            return 2;
        }
        if (id > 2000)
        {
            return 3;
        }
        return 0;
    }

    void ReportError(const Error &error, const SourceManager &srcManager);
}; // namespace Fig