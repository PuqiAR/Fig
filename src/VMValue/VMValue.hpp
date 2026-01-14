#pragma once

/*
    value system for virtual machine
    not Evaluator value
*/

#include <Core/fig_string.hpp>

#include <cstdint>

namespace Fig
{
    struct HeapObject;

    struct Value
    {
        struct NullObject {};

        enum _Type : uint8_t
        {
            Null = 0,
            Bool,
            Int,
            Double,

            /* Complex types, alloc in heap*/
            String,
            // Function,
            // StructType,
            // StructInstance,
            // List,
            // Map,
            // Module,
            // InterfaceType
        } type;
        
        union
        {
            NullObject n;
            bool b;
            int64_t i;
            double d;

            HeapObject *heap;
        };

        Value()
        {
            type = Null;
            n = NullObject();
        }

        Value(bool _b)
        {
            type = Bool;
            b = _b;
        }

        Value(int64_t _i)
        {
            type = Int;
            i = _i;
        }

        Value(double _d)
        {
            type = Double;
            d = _d;
        }

        FString toString() const;
    };

    struct HeapObject
    {
        Value::_Type type; // > 3

        virtual FString toString() const = 0;
    };

    struct String : HeapObject
    {
        FString value;

        virtual FString toString() const override
        {
            return value;
        }
    };
};