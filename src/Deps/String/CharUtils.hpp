#pragma once

namespace Fig::Deps
{
    class CharUtils
    {
    public:
        using U32 = char32_t;

        // ===== 基础 =====

        static constexpr bool isValidScalar(U32 c) noexcept { return c <= 0x10FFFF && !(c >= 0xD800 && c <= 0xDFFF); }

        static constexpr bool isAscii(U32 c) noexcept { return c <= 0x7F; }

        static constexpr bool isControl(U32 c) noexcept { return (c <= 0x1F) || (c == 0x7F); }

        static constexpr bool isPrintable(U32 c) noexcept { return !isControl(c); }

        // ===== ASCII 分类 =====

        static constexpr bool isAsciiLower(U32 c) noexcept { return c >= U'a' && c <= U'z'; }
        static constexpr bool isAsciiUpper(U32 c) noexcept { return c >= U'A' && c <= U'Z'; }
        static constexpr bool isAsciiAlpha(U32 c) noexcept { return isAsciiLower(c) || isAsciiUpper(c); }
        static constexpr bool isAsciiDigit(U32 c) noexcept { return c >= U'0' && c <= U'9'; }

        static constexpr bool isAsciiHexDigit(U32 c) noexcept
        {
            return isAsciiDigit(c) || (c >= U'a' && c <= U'f') || (c >= U'A' && c <= U'F');
        }

        static constexpr bool isAsciiSpace(U32 c) noexcept { return c == U' ' || (c >= 0x09 && c <= 0x0D); }

        static constexpr bool isAsciiPunct(U32 c) noexcept
        {
            return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
        }

        // ===== Unicode White_Space =====

        static constexpr bool isSpace(U32 c) noexcept
        {
            if (isAscii(c)) return isAsciiSpace(c);

            switch (c)
            {
                case 0x0085:
                case 0x00A0:
                case 0x1680:
                case 0x2000:
                case 0x2001:
                case 0x2002:
                case 0x2003:
                case 0x2004:
                case 0x2005:
                case 0x2006:
                case 0x2007:
                case 0x2008:
                case 0x2009:
                case 0x200A:
                case 0x2028:
                case 0x2029:
                case 0x202F:
                case 0x205F:
                case 0x3000: return true;
            }
            return false;
        }

        // ===== Unicode Decimal_Number =====

        static constexpr bool isDigit(U32 c) noexcept
        {
            if (isAscii(c)) return isAsciiDigit(c);

            return (c >= 0x0660 && c <= 0x0669) || (c >= 0x06F0 && c <= 0x06F9) || (c >= 0x0966 && c <= 0x096F)
                   || (c >= 0x09E6 && c <= 0x09EF) || (c >= 0x0A66 && c <= 0x0A6F) || (c >= 0x0AE6 && c <= 0x0AEF)
                   || (c >= 0x0B66 && c <= 0x0B6F) || (c >= 0x0BE6 && c <= 0x0BEF) || (c >= 0x0C66 && c <= 0x0C6F)
                   || (c >= 0x0CE6 && c <= 0x0CEF) || (c >= 0x0D66 && c <= 0x0D6F) || (c >= 0x0E50 && c <= 0x0E59)
                   || (c >= 0x0ED0 && c <= 0x0ED9) || (c >= 0x0F20 && c <= 0x0F29) || (c >= 0x1040 && c <= 0x1049)
                   || (c >= 0x17E0 && c <= 0x17E9) || (c >= 0x1810 && c <= 0x1819) || (c >= 0xFF10 && c <= 0xFF19);
        }

        // ===== Unicode Letter =====

        static constexpr bool isAlpha(U32 c) noexcept
        {
            if (isAscii(c)) return isAsciiAlpha(c);

            return (c >= 0x00C0 && c <= 0x02AF) || (c >= 0x0370 && c <= 0x052F) || (c >= 0x0530 && c <= 0x058F)
                   || (c >= 0x0590 && c <= 0x05FF) || (c >= 0x0600 && c <= 0x06FF) || (c >= 0x0900 && c <= 0x097F)
                   || (c >= 0x3040 && c <= 0x30FF) || (c >= 0x3100 && c <= 0x312F) || (c >= 0x4E00 && c <= 0x9FFF)
                   || (c >= 0xAC00 && c <= 0xD7AF);
        }

        // ===== 标点 / 符号 / 分隔符（工程近似）=====

        static constexpr bool isPunct(U32 c) noexcept
        {
            if (isAscii(c)) return isAsciiPunct(c);
            return (c >= 0x2000 && c <= 0x206F);
        }

        static constexpr bool isSymbol(U32 c) noexcept
        {
            return (c >= 0x20A0 && c <= 0x20CF) || // currency
                   (c >= 0x2100 && c <= 0x214F) || // letterlike
                   (c >= 0x2190 && c <= 0x21FF) || // arrows
                   (c >= 0x2600 && c <= 0x26FF) || // misc symbols
                   (c >= 0x1F300 && c <= 0x1FAFF); // emoji block
        }

        // ===== 组合 =====

        static constexpr bool isAlnum(U32 c) noexcept { return isAlpha(c) || isDigit(c); }

        static constexpr bool isHexDigit(U32 c) noexcept { return isAsciiHexDigit(c); }

        static constexpr bool isIdentifierStart(U32 c) noexcept { return isAlpha(c) || c == U'_'; }

        static constexpr bool isIdentifierContinue(U32 c) noexcept { return isAlnum(c) || c == U'_'; }
    };
};