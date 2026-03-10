/*!
    @file src/Error/Diagnostics.hpp
    @brief 诊断信息收集器：用于存储警告与非致命错误
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#pragma once

#include <Error/Error.hpp>

namespace Fig
{
    class Diagnostics
    {
    private:
        DynArray<Error> errors;

    public:
        void Report(Error err)
        {
            errors.push_back(std::move(err));
        }

        // 统一打印所有收集到的信息
        void EmitAll(const SourceManager &manager)
        {
            for (const auto &err : errors)
            {
                ReportError(err, manager);
            }
        }

        bool HasErrors() const
        {
            for (const auto &err : errors)
            {
                if (!err.IsWarning()) return true;
            }
            return false;
        }

        const DynArray<Error>& GetErrors() const { return errors; }
        
        void Clear() { errors.clear(); }
    };
} // namespace Fig
