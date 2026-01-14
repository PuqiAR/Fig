#include <VMValue/VMValue.hpp>

#include <cassert>
#include <string>

namespace Fig
{
    FString Value::toString() const
    {
        if (static_cast<int>(type) > static_cast<int>(Double))
        {
            return heap->toString();
        }
        switch (type)
        {
            case Null:
                return u8"null";
            case Bool:
                return (b ? u8"true" : u8"false");
            case Int:
                return FString(std::to_string(i));
            case Double:
                return FString(std::to_string(d));
            
            default:
                assert(false);
        }
    }
};