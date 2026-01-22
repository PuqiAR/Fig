#pragma once


#include <Ast/AccessModifier.hpp>
#include <Core/fig_string.hpp>
#include <Evaluator/Value/Type.hpp>
#include <Evaluator/Value/value_forward.hpp>

#include <memory>
namespace Fig
{

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