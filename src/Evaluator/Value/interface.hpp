#pragma once

#include <Ast/Statements/InterfaceDefSt.hpp>
#include <Value/Type.hpp>

#include <vector>

namespace Fig
{
    struct InterfaceType
    {
        TypeInfo type;
        std::vector<Ast::InterfaceMethod> methods;

        bool operator==(const InterfaceType &other) const
        {
            return type == other.type; // only compare type info (chain -> typeinfo.id)
        }
    };
}