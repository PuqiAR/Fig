#pragma once


#include <Ast/AccessModifier.hpp>
#include <Core/fig_string.hpp>
#include <Value/Type.hpp>
#include <memory>
namespace Fig
{
    class Object;
    using ObjectPtr = std::shared_ptr<Object>;
    struct VariableSlot
    {
        FString name;
        ObjectPtr value;
        TypeInfo declaredType;
        AccessModifier am;

        bool isRef = false;
        std::shared_ptr<VariableSlot> refTarget;
    };
}