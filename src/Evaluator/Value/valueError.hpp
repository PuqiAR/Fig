#pragma once

#include <Error/error.hpp>

namespace Fig
{
    class ValueError : public UnaddressableError
    {
    public:
        using UnaddressableError::UnaddressableError;
        virtual FString toString() const override
        {
            std::string msg = std::format("[ValueError] {} in [{}] {}", std::string(this->message.begin(), this->message.end()), this->src_loc.file_name(), this->src_loc.function_name());
            return FString(msg);
        }
    };
};