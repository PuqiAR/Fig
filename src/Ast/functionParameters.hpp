#pragma once

#include <Ast/astBase.hpp>
#include <Value/Type.hpp>
#include <Core/fig_string.hpp>

namespace Fig::Ast
{
    struct FunctionParameters // for define
    {
        /*
            Positional Parameters:
                func test(pp1, pp2: Int)
            Default Parameters:
                func test2(dp1 = 10, dp2:String = "default parameter 2")
        */

        using PosParasType = std::vector<std::pair<FString, FString>>;
        using DefParasType = std::vector<std::pair<FString, std::pair<FString, Expression>>>;

        PosParasType posParas;
        DefParasType defParas; // default parameters

        FString variadicPara;
        bool variadic = false;

        FunctionParameters()
        {
            
        }
        FunctionParameters(PosParasType _posParas, DefParasType _defParas)
        {
            posParas = std::move(_posParas);
            defParas = std::move(_defParas);
        }
        FunctionParameters(FString _variadicPara)
        {
            variadicPara = std::move(_variadicPara);
            variadic = true;
        }

        size_t size() const
        {
            return posParas.size() + defParas.size();
        }
    };
}