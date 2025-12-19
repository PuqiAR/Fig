#pragma once
#pragma once
#include <fig_string.hpp>
#include <string>
#include <locale>
#include <cwctype>
#include <vector>
#include <algorithm>

namespace Fig::Utils
{
    inline std::u32string utf8ToUtf32(const FString &s)
    {
        std::u32string result;
        size_t i = 0;
        while (i < s.size())
        {
            char32_t codepoint = 0;
            unsigned char c = static_cast<unsigned char>(s[i]);

            if (c < 0x80)
            {
                codepoint = c;
                i += 1;
            }
            else if ((c >> 5) == 0x6)
            {
                codepoint = ((c & 0x1F) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3F);
                i += 2;
            }
            else if ((c >> 4) == 0xE)
            {
                codepoint = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i + 2]) & 0x3F);
                i += 3;
            }
            else if ((c >> 3) == 0x1E)
            {
                codepoint = ((c & 0x07) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3F) << 12) | ((static_cast<unsigned char>(s[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(s[i + 3]) & 0x3F);
                i += 4;
            }
            else
            {
                i += 1; // 跳过非法字节
                continue;
            }
            result.push_back(codepoint);
        }
        return result;
    }

    inline FString utf32ToUtf8(const std::u32string &s)
    {
        FString result;
        for (char32_t cp : s)
        {
            if (cp < 0x80)
            {
                result.push_back(static_cast<char8_t>(cp));
            }
            else if (cp < 0x800)
            {
                result.push_back(static_cast<char8_t>((cp >> 6) | 0xC0));
                result.push_back(static_cast<char8_t>((cp & 0x3F) | 0x80));
            }
            else if (cp < 0x10000)
            {
                result.push_back(static_cast<char8_t>((cp >> 12) | 0xE0));
                result.push_back(static_cast<char8_t>(((cp >> 6) & 0x3F) | 0x80));
                result.push_back(static_cast<char8_t>((cp & 0x3F) | 0x80));
            }
            else
            {
                result.push_back(static_cast<char8_t>((cp >> 18) | 0xF0));
                result.push_back(static_cast<char8_t>(((cp >> 12) & 0x3F) | 0x80));
                result.push_back(static_cast<char8_t>(((cp >> 6) & 0x3F) | 0x80));
                result.push_back(static_cast<char8_t>((cp & 0x3F) | 0x80));
            }
        }
        return result;
    }

    inline FString toLower(const FString &s)
    {
        std::u32string u32 = utf8ToUtf32(s);
        std::locale loc("");
        for (auto &ch : u32)
        {
            ch = std::towlower(ch);
        }
        return utf32ToUtf8(u32);
    }

    inline FString toUpper(const FString &s)
    {
        std::u32string u32 = utf8ToUtf32(s);
        std::locale loc("");
        for (auto &ch : u32)
        {
            ch = std::towupper(ch);
        }
        return utf32ToUtf8(u32);
    }

    template <class T>
    bool vectorContains(const T &t, const std::vector<T> v)
    {
        return std::find(v.begin(), v.end(), t) != v.end();
    }
} // namespace Fig::Utils
