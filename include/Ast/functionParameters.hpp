#pragma once

#include <Ast/astBase.hpp>
#include <Value/Type.hpp>
#include <fig_string.hpp>

namespace Fig::Ast
{
    struct FunctionParameters // for define
    {
        /*
            Positional Parameters:
                fun test(pp1, pp2: Int)
            Default Parameters:
                fun test2(dp1 = 10, dp2:String = "default parameter 2")
        */

        using PosParasType = std::vector<std::pair<FString, FString>>;
        using DefParasType = std::vector<std::pair<FString, std::pair<FString, Expression>>>;

        PosParasType posParas;
        DefParasType defParas; // default parameters

        FunctionParameters()
        {
            
        }
        FunctionParameters(PosParasType _posParas, DefParasType _defParas)
        {
            posParas = std::move(_posParas);
            defParas = std::move(_defParas);
        }

        size_t size() const
        {
            return posParas.size() + defParas.size();
        }
    };
}