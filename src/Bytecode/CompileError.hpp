#pragma once

#include <Error/error.hpp>

namespace Fig
{
    class CompileError : public AddressableError
    {
        using AddressableError::AddressableError;

        virtual FString toString() const override
        {
            std::string msg = std::format("[CompileError] {} in [{}] {}",
                                          this->message.toBasicString(),
                                          this->src_loc.file_name(),
                                          this->src_loc.function_name());
            return FString(msg);
        }

        virtual FString getErrorType() const override { return FString(u8"CompileError"); }
    };
};