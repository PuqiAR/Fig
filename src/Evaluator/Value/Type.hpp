#pragma once

#include <Core/fig_string.hpp>

#include <variant>
#include <map>

namespace Fig
{

    class TypeInfo final
    {
    private:
        size_t id;

    public:
        friend class TypeInfoHash;

        FString name;

        FString toString() const
        {
            return name;
        }

        static std::map<FString, size_t> typeMap;

        static size_t getID(FString _name)
        {
            return typeMap.at(_name);
        }
        size_t getInstanceID() const
        {
            return id;
        }

        TypeInfo();
        explicit TypeInfo(const FString &_name, bool reg = false);
        TypeInfo(const TypeInfo &other) = default;

        bool operator==(const TypeInfo &other) const
        {
            return id == other.id;
        }
    };
    
    class TypeInfoHash
    {
    public:
        std::size_t operator()(const TypeInfo &ti) const
        {
            return std::hash<size_t>{}(ti.id);
        }
    };

    // class Value;
    namespace ValueType
    {
        extern const TypeInfo Any;
        extern const TypeInfo Null;
        extern const TypeInfo Int;
        extern const TypeInfo String;
        extern const TypeInfo Bool;
        extern const TypeInfo Double;
        extern const TypeInfo Function;
        extern const TypeInfo StructType;
        extern const TypeInfo StructInstance;
        extern const TypeInfo List;
        extern const TypeInfo Map;
        extern const TypeInfo Module;
        extern const TypeInfo InterfaceType;

        using IntClass = int64_t;
        using DoubleClass = double;
        using BoolClass = bool;
        using NullClass = std::monostate;
        using StringClass = FString;
    }; // namespace ValueType
}; // namespace Fig


namespace std
{
    template <>
    struct hash<Fig::TypeInfo>
    {
        size_t operator()(const Fig::TypeInfo &t)
        {
            return std::hash<size_t>{}(t.getInstanceID());
        }
    };
};