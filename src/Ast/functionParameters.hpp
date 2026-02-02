#pragma once

#include <Ast/astBase.hpp>
#include <Evaluator/Value/Type.hpp>
#include <Core/fig_string.hpp>

#include <format>

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

        using PosParasType = std::vector<std::pair<FString, Expression>>;
        //                                          name       type exp
        using DefParasType = std::vector<std::pair<FString, std::pair<Expression, Expression>>>;
        //                                          name                type exp     default value

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

        bool operator==(const FunctionParameters &other) const
        {
            return posParas == other.posParas && defParas == other.defParas && variadicPara == other.variadicPara && variadic == other.variadic;
        }

        FString toString() const
        {
            if (variadic)
            {
                return FString(variadicPara + u8"...");
            }
            const auto posParasToString = [this]() {
                FString out;
                for (auto &p : posParas)
                {
                    out += p.first;
                    if (p.second != nullptr)
                    {
                        out += FString(u8":" + p.second->toString());
                    }
                    out += u8",";
                }
                out.pop_back();
                return out;
            };
            const auto defParasToString = [this]() {
                FString out;
                for (auto &p : defParas)
                {
                    out += p.first;
                    if (p.second.first != nullptr)
                    {
                        out += FString(u8":" + p.second.first->toString());
                    }
                    if (p.second.second != nullptr)
                    {
                        out += u8"=";
                        out += p.second.second->toString();
                    }
                    out += u8",";
                }
                out.pop_back();
                return out;
            };
            return FString(std::format("{},{}", posParasToString().toBasicString(), defParasToString().toBasicString()));
        }
    };
} // namespace Fig::Ast