#pragma once

#include <error.hpp>
#include <core.hpp>

#include <print>
#include <vector>


namespace Fig
{
    namespace ErrorLog
    {
        namespace TerminalColors
        {
            constexpr const char *Reset = "\033[0m";
            constexpr const char *Bold = "\033[1m";
            constexpr const char *Dim = "\033[2m";
            constexpr const char *Italic = "\033[3m";
            constexpr const char *Underline = "\033[4m";
            constexpr const char *Blink = "\033[5m";
            constexpr const char *Reverse = "\033[7m"; // 前背景反色
            constexpr const char *Hidden = "\033[8m";  // 隐藏文本
            constexpr const char *Strike = "\033[9m";  // 删除线

            constexpr const char *Black = "\033[30m";
            constexpr const char *Red = "\033[31m";
            constexpr const char *Green = "\033[32m";
            constexpr const char *Yellow = "\033[33m";
            constexpr const char *Blue = "\033[34m";
            constexpr const char *Magenta = "\033[35m";
            constexpr const char *Cyan = "\033[36m";
            constexpr const char *White = "\033[37m";

            constexpr const char *LightBlack = "\033[90m";
            constexpr const char *LightRed = "\033[91m";
            constexpr const char *LightGreen = "\033[92m";
            constexpr const char *LightYellow = "\033[93m";
            constexpr const char *LightBlue = "\033[94m";
            constexpr const char *LightMagenta = "\033[95m";
            constexpr const char *LightCyan = "\033[96m";
            constexpr const char *LightWhite = "\033[97m";

            constexpr const char *DarkRed = "\033[38;2;128;0;0m";
            constexpr const char *DarkGreen = "\033[38;2;0;100;0m";
            constexpr const char *DarkYellow = "\033[38;2;128;128;0m";
            constexpr const char *DarkBlue = "\033[38;2;0;0;128m";
            constexpr const char *DarkMagenta = "\033[38;2;100;0;100m";
            constexpr const char *DarkCyan = "\033[38;2;0;128;128m";
            constexpr const char *DarkGray = "\033[38;2;64;64;64m";
            constexpr const char *Gray = "\033[38;2;128;128;128m";
            constexpr const char *Silver = "\033[38;2;192;192;192m";

            constexpr const char *Navy = "\033[38;2;0;0;128m";
            constexpr const char *RoyalBlue = "\033[38;2;65;105;225m";
            constexpr const char *ForestGreen = "\033[38;2;34;139;34m";
            constexpr const char *Olive = "\033[38;2;128;128;0m";
            constexpr const char *Teal = "\033[38;2;0;128;128m";
            constexpr const char *Maroon = "\033[38;2;128;0;0m";
            constexpr const char *Purple = "\033[38;2;128;0;128m";
            constexpr const char *Orange = "\033[38;2;255;165;0m";
            constexpr const char *Gold = "\033[38;2;255;215;0m";
            constexpr const char *Pink = "\033[38;2;255;192;203m";
            constexpr const char *Crimson = "\033[38;2;220;20;60m";

            constexpr const char *OnBlack = "\033[40m";
            constexpr const char *OnRed = "\033[41m";
            constexpr const char *OnGreen = "\033[42m";
            constexpr const char *OnYellow = "\033[43m";
            constexpr const char *OnBlue = "\033[44m";
            constexpr const char *OnMagenta = "\033[45m";
            constexpr const char *OnCyan = "\033[46m";
            constexpr const char *OnWhite = "\033[47m";

            constexpr const char *OnLightBlack = "\033[100m";
            constexpr const char *OnLightRed = "\033[101m";
            constexpr const char *OnLightGreen = "\033[102m";
            constexpr const char *OnLightYellow = "\033[103m";
            constexpr const char *OnLightBlue = "\033[104m";
            constexpr const char *OnLightMagenta = "\033[105m";
            constexpr const char *OnLightCyan = "\033[106m";
            constexpr const char *OnLightWhite = "\033[107m";

            constexpr const char *OnDarkBlue = "\033[48;2;0;0;128m";
            constexpr const char *OnGreenYellow = "\033[48;2;173;255;47m";
            constexpr const char *OnOrange = "\033[48;2;255;165;0m";
            constexpr const char *OnGray = "\033[48;2;128;128;128m";
        }; // namespace TerminalColors

        inline void coloredPrint(const char *colorCode, FString msg)
        {
            std::print("{}{}{}", colorCode, msg.toBasicString(), TerminalColors::Reset);
        }

        inline void coloredPrint(const char *colorCode, std::string msg)
        {
            std::print("{}{}{}", colorCode, msg, TerminalColors::Reset);
        }


        inline void logAddressableError(const AddressableError &err, FString fileName, std::vector<FString> sourceLines)
        {
            std::print("\n");
            namespace TC = TerminalColors;
            coloredPrint(TC::LightWhite, "An error occurred! ");
            coloredPrint(TC::White, std::format("Fig {} ({})[{} {} bit on `{}`]\n",Core::VERSION, Core::COMPILE_TIME, Core::COMPILER, Core::ARCH, Core::PLATFORM));
            coloredPrint(TC::LightRed, "✖  ");
            coloredPrint(TC::LightRed, std::format("{}: {}\n", err.getErrorType().toBasicString(), FString(err.getMessage()).toBasicString()));
            coloredPrint(TC::White, std::format("    at {}:{} in file '{}'\n", err.getLine(), err.getColumn(), fileName.toBasicString()));
            FString lineContent = ((int64_t(err.getLine()) - int64_t(1)) >= 0 ? sourceLines[err.getLine() - 1] : u8"<No Source>");
            coloredPrint(TC::LightBlue, std::format("    {}\n", lineContent.toBasicString()));
            FString pointerLine;
            for (size_t i = 1; i < err.getColumn(); ++i)
            {
                if (lineContent[i - 1] == U'\t')
                {
                    pointerLine += U'\t';
                }
                else
                {
                    pointerLine += U' ';
                }
            }
            pointerLine += U'^';
            coloredPrint(TC::LightGreen, std::format("    {}\n", pointerLine.toBasicString()));
            coloredPrint(TC::DarkGray, std::format("🔧 in function '{}' ({}:{})\n", err.src_loc.function_name(), err.src_loc.file_name(), err.src_loc.line()));
        }

        inline void logUnaddressableError(const UnaddressableError &err)
        {
            std::print("\n");
            namespace TC = TerminalColors;
            coloredPrint(TC::LightWhite, "An error occurred! ");
            coloredPrint(TC::White, std::format("Fig {} ({})[{} {} bit on `{}`]\n", Core::VERSION, Core::COMPILE_TIME, Core::COMPILER, Core::ARCH, Core::PLATFORM));
            coloredPrint(TC::DarkRed, "✖");
            coloredPrint(TC::Red, std::format("{}: {}\n", err.getErrorType().toBasicString(), FString(err.getMessage()).toBasicString()));
            coloredPrint(TC::DarkGray, std::format("🔧 in function '{}' ({}:{})", err.src_loc.function_name(), err.src_loc.file_name(), err.src_loc.line()));
        }
    }; // namespace ErrorLog
}; // namespace Fig