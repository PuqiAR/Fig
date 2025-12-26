#pragma once

#include <Ast/astBase.hpp>

#include <vector>

namespace Fig::Ast
{
    class ImportSt final : public StatementAst
    {
    public:
        std::vector<FString> path;
        
        ImportSt()
        {
            type = AstType::ImportSt;
        }

        ImportSt(std::vector<FString> _path) :
            path(std::move(_path)) 
        {
            type = AstType::ImportSt;
        }
    };

    using Import = std::shared_ptr<ImportSt>;
};