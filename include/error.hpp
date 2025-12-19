#pragma once

#include <fig_string.hpp>

#include <exception>
#include <format>
#include <source_location>
#include <string>

namespace Fig
{
    class AddressableError : public std::exception
    {
    public:
        explicit AddressableError() {}
        explicit AddressableError(FStringView _msg,
                                  size_t _line,
                                  size_t _column,
                                  std::source_location loc = std::source_location::current()) :
            src_loc(loc), line(_line), column(_column)
        {
            message = _msg;
        }
        virtual FString toString() const
        {
            std::string msg = std::format("[AddressableError] {} at {}:{}, in [{}] {}", std::string(this->message.begin(), this->message.end()), this->line, this->column, this->src_loc.file_name(), this->src_loc.function_name());
            return FString(msg);
        }
        const char *what() const noexcept override
        {
            static std::string msg = toString().toBasicString();
            return msg.c_str();
        }
        std::source_location src_loc;

        size_t getLine() const { return line; }
        size_t getColumn() const { return column; }
        FStringView getMessage() const { return message; }

        virtual FString getErrorType() const
        {
            return FString(u8"AddressableError");
        }

    protected:
        size_t line, column;
        FStringView message;
    };

    class UnaddressableError : public std::exception
    {
    public:
        explicit UnaddressableError() {}
        explicit UnaddressableError(FStringView _msg,
                                    std::source_location loc = std::source_location::current()) :
            src_loc(loc)
        {
            message = _msg;
        }
        virtual FString toString() const
        {
            std::string msg = std::format("[UnaddressableError] {} in [{}] {}", std::string(this->message.begin(), this->message.end()),  this->src_loc.file_name(), this->src_loc.function_name());
            return FString(msg);
        }
        const char *what() const noexcept override
        {
            static std::string msg = toString().toBasicString();
            return msg.c_str();
        }
        std::source_location src_loc;
        FStringView getMessage() const { return message; }

        virtual FString getErrorType() const
        {
            return FString(u8"UnaddressableError");
        }

    protected:
        FStringView message;
    };

    class SyntaxError : public AddressableError
    {
    public:
        using AddressableError::AddressableError;

        explicit SyntaxError(FStringView _msg,
                             size_t _line,
                             size_t _column,
                             std::source_location loc = std::source_location::current()) :
            AddressableError(_msg, _line, _column, loc)
        {
        }

        virtual FString toString() const override
        {
            std::string msg = std::format("[SyntaxError] {} in [{}] {}", std::string(this->message.begin(), this->message.end()), this->src_loc.file_name(), this->src_loc.function_name());
            return FString(msg);
        }

        virtual FString getErrorType() const override
        {
            return FString(u8"SyntaxError");
        }
    };

    class RuntimeError final : public UnaddressableError
    {
    public:
        using UnaddressableError::UnaddressableError;
        explicit RuntimeError(FStringView _msg,
                             std::source_location loc = std::source_location::current()) :
            UnaddressableError(_msg, loc)
        {
        }
        virtual FString toString() const override
        {
            std::string msg = std::format("[RuntimeError] {} in [{}] {}", std::string(this->message.begin(), this->message.end()), this->src_loc.file_name(), this->src_loc.function_name());
            return FString(msg);
        }

        virtual FString getErrorType() const override
        {
            return FString(u8"RuntimeError");
        }
    };
    
} // namespace Fig