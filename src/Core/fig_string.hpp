#pragma once
#include <string>
#include <string_view>


namespace Fig
{
    // using String = std::u8string;
    // using StringView = std::u8string_view;

    class FStringView : public std::u8string_view
    {
    public:
        using std::u8string_view::u8string_view;
        
        static FStringView fromBasicStringView(std::string_view sv)
        {
            return FStringView(reinterpret_cast<const char8_t*>(sv.data()));
        }

        explicit FStringView(std::string_view sv)
        {
            *this = fromBasicStringView(sv);
        }

        explicit FStringView()
        {
            *this = fromBasicStringView(std::string_view(""));
        }

        std::string_view toBasicStringView() const
        {
            return std::string_view(reinterpret_cast<const char *>(data()), size());
        }
    };

    class FString : public std::u8string
    {
    public:
        using std::u8string::u8string;

        FString operator+(const FString& x)
        {
            return FString(toBasicString() + x.toBasicString());
        }

        explicit FString(const std::u8string &str)
        {
            *this = fromU8String(str);
        }
        explicit FString(std::string str)
        {
            *this = fromBasicString(str);
        }
        explicit FString(FStringView sv)
        {
            *this = fromStringView(sv);
        }
        std::string toBasicString() const
        {
            return std::string(this->begin(), this->end());
        }
        FStringView toStringView() const
        {
            return FStringView(this->data(), this->size());
        }

        static FString fromBasicString(const std::string &str)
        {
            return FString(str.begin(), str.end());
        }

        static FString fromStringView(FStringView sv)
        {
            return FString(reinterpret_cast<const char*> (sv.data()));
        }

        static FString fromU8String(const std::u8string &str)
        {
            return FString(str.begin(), str.end());
        }

        size_t length() const
        {
            // get UTF8-String real length
            size_t len = 0;
            for (auto it = this->begin(); it != this->end(); ++it)
            {
                if ((*it & 0xC0) != 0x80)
                {
                    ++len;
                }
            }
            return len;
        }
    };

}; // namespace Fig

namespace std
{
    template <>
    struct hash<Fig::FString>
    {
        std::size_t operator()(const Fig::FString &s) const
        {
            return std::hash<std::u8string>{}(static_cast<const std::u8string &>(s));
        }
    };
}