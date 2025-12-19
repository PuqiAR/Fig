#include <warning.hpp>

namespace Fig
{
    const std::unordered_map<std::size_t, FString> Warning::standardWarnings = {
        {1, FString(u8"Identifier is too similar to a keyword or a primitive type")},
        {2, FString(u8"The identifier is too abstract")}
    };
};