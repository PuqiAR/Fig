// #include <corecrt.h>
#include <string>
#include <iterator>
#include <string>
#include <cwctype>
// fuckyou C++
// i don't know how to deal with unicode string in cpp
// fuck
// generate by Qwen3-Coder:

// 2025-12-26
/*
again: C++ fuck you
fuck you
fuck you!

fuck

C++ IS A PIECE OF SHIT
STOP USING C++

unicodeString = "some unicode points";
unicodeString[0] ---> ?????
FUCK YOU C++
*/

namespace Fig
{
    class UTF8Char
    {
    private:
        std::u8string char_data_;

    public:
        UTF8Char(const std::u8string &data) :
            char_data_(data) {}

        // 获取UTF-8字符的字节长度
        static size_t getUTF8CharLength(char8_t first_byte)
        {
            if ((first_byte & 0x80) == 0x00) return 1;
            if ((first_byte & 0xE0) == 0xC0) return 2;
            if ((first_byte & 0xF0) == 0xE0) return 3;
            if ((first_byte & 0xF8) == 0xF0) return 4;
            return 1;
        }

        // 转换为Unicode码点
        char32_t toCodePoint() const
        {
            if (char_data_.empty()) return 0;

            size_t len = getUTF8CharLength(char_data_[0]);
            if (len > char_data_.length()) return 0;

            char32_t code_point = 0;

            switch (len)
            {
                case 1:
                    code_point = char_data_[0];
                    break;
                case 2:
                    code_point = ((char_data_[0] & 0x1F) << 6) | (char_data_[1] & 0x3F);
                    break;
                case 3:
                    code_point = ((char_data_[0] & 0x0F) << 12) | ((char_data_[1] & 0x3F) << 6) | (char_data_[2] & 0x3F);
                    break;
                case 4:
                    code_point = ((char_data_[0] & 0x07) << 18) | ((char_data_[1] & 0x3F) << 12) | ((char_data_[2] & 0x3F) << 6) | (char_data_[3] & 0x3F);
                    break;
            }
            return code_point;
        }

        inline bool operator==(char32_t ch)
        {
            return this->toCodePoint() == ch;
        }
        // 字符分类函数
        bool isAlpha() const
        {
            char32_t cp = toCodePoint();
            return std::iswalpha(static_cast<wint_t>(cp));
        }

        bool isDigit() const
        {
            char32_t cp = toCodePoint();
            return std::iswdigit(static_cast<wint_t>(cp));
        }

        bool isAlnum() const
        {
            char32_t cp = toCodePoint();
            return std::iswalnum(static_cast<wint_t>(cp));
        }

        bool isSpace() const
        {
            char32_t cp = toCodePoint();
            return std::iswspace(static_cast<wint_t>(cp));
        }

        bool isUpper() const
        {
            char32_t cp = toCodePoint();
            return std::iswupper(static_cast<wint_t>(cp));
        }

        bool isLower() const
        {
            char32_t cp = toCodePoint();
            return std::iswlower(static_cast<wint_t>(cp));
        }

        bool isPunct() const
        {
            char32_t cp = toCodePoint();
            return std::iswpunct(static_cast<wint_t>(cp));
        }

        // 获取底层数据
        const std::u8string &getString() const { return char_data_; }

        // 获取字符长度（字节数）
        size_t length() const { return char_data_.length(); }

        // 是否为空
        bool empty() const { return char_data_.empty(); }
    };

    class UTF8Iterator
    {
    private:
        const std::u8string *str_;
        size_t pos_;

        // 获取UTF-8字符的字节长度
        static size_t getUTF8CharLength(char8_t first_byte)
        {
            if ((first_byte & 0x80) == 0x00) return 1;
            if ((first_byte & 0xE0) == 0xC0) return 2;
            if ((first_byte & 0xF0) == 0xE0) return 3;
            if ((first_byte & 0xF8) == 0xF0) return 4;
            return 1;
        }

        // 获取下一个字符的起始位置
        size_t getNextCharPos(size_t current_pos) const
        {
            if (current_pos >= str_->length()) return current_pos;
            size_t char_len = getUTF8CharLength((*str_)[current_pos]);
            return current_pos + char_len;
        }

        // 获取前一个字符的起始位置
        size_t getPrevCharPos(size_t current_pos) const
        {
            if (current_pos == 0) return 0;

            size_t pos = current_pos - 1;
            while (pos > 0 && (str_->at(pos) & 0xC0) == 0x80)
            {
                --pos;
            }
            return pos;
        }

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = UTF8Char;
        using difference_type = std::ptrdiff_t;
        using pointer = const UTF8Char *;
        using reference = const UTF8Char &;

        // 构造函数
        UTF8Iterator(const std::u8string &str, size_t pos = 0) :
            str_(&str), pos_(pos)
        {
            if (pos_ > str_->length()) pos_ = str_->length();
        }

        // 前置递增
        UTF8Iterator &operator++()
        {
            pos_ = getNextCharPos(pos_);
            return *this;
        }

        // 后置递增
        UTF8Iterator operator++(int)
        {
            UTF8Iterator temp = *this;
            pos_ = getNextCharPos(pos_);
            return temp;
        }

        // 前置递减
        UTF8Iterator &operator--()
        {
            pos_ = getPrevCharPos(pos_);
            return *this;
        }

        // 后置递减
        UTF8Iterator operator--(int)
        {
            UTF8Iterator temp = *this;
            pos_ = getPrevCharPos(pos_);
            return temp;
        }

        // 解引用操作符 - 返回当前字符
        UTF8Char operator*() const
        {
            if (pos_ >= str_->length())
            {
                return UTF8Char(std::u8string());
            }
            size_t char_len = getUTF8CharLength((*str_)[pos_]);
            size_t end_pos = pos_ + char_len;
            if (end_pos > str_->length())
            {
                end_pos = str_->length();
            }
            return UTF8Char(str_->substr(pos_, end_pos - pos_));
        }
        UTF8Char peek() const
        {
            if (pos_ >= str_->length())
            {
                return UTF8Char(std::u8string());
            }

            size_t next_pos = getNextCharPos(pos_);
            if (next_pos >= str_->length())
            {
                return UTF8Char(std::u8string());
            }

            size_t char_len = getUTF8CharLength((*str_)[next_pos]);
            size_t end_pos = next_pos + char_len;
            if (end_pos > str_->length())
            {
                end_pos = str_->length();
            }

            return UTF8Char(str_->substr(next_pos, end_pos - next_pos));
        }

        // 窥探前一个字符
        UTF8Char peekPrev() const
        {
            if (pos_ == 0)
            {
                return UTF8Char(std::u8string());
            }

            size_t prev_pos = getPrevCharPos(pos_);
            size_t char_len = getUTF8CharLength((*str_)[prev_pos]);
            size_t end_pos = prev_pos + char_len;
            if (end_pos > str_->length())
            {
                end_pos = str_->length();
            }

            return UTF8Char(str_->substr(prev_pos, end_pos - prev_pos));
        }
        // 获取当前位置
        size_t position() const { return pos_; }
        size_t column() const { return pos_ + 1; }
        // 检查是否到达末尾
        bool isEnd() const { return pos_ >= str_->length(); }
    };
} // namespace Fig