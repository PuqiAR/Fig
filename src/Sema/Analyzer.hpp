/*!
    @file src/Sema/Analyzer.hpp
    @brief 语义分析
    @author PuqiAR (im@puqiar.top)
    @date 2026-06-06
*/

#pragma once

#include <Ast/Ast.hpp>
#include <Sema/Type.hpp>
#include <Error/Diagnostics.hpp>
#include <SourceManager/SourceManager.hpp>

namespace Fig
{
    class Analyzer
    {
    public:
        Analyzer(SourceManager &) {}

        Result<void, Error> Analyze(Program *)
        {
            return {};
        }

        Diagnostics &GetDiagnostics() { return diag; }

    private:
        Diagnostics diag;
    };
} // namespace Fig
