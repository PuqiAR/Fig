/*!
    @file src/Ast/Stmt/ImportStmt.hpp
    @brief import
    @author PuqiAR (im@puqiar.top)
    @date 2026-06-06
*/

#pragma once

#include <Ast/Base.hpp>

namespace Fig
{
    struct ImportStmt final : public Stmt
    {
        String path;
        bool   isFileImport;

        ImportStmt() { type = AstType::ImportStmt; }

        ImportStmt(String _path, bool _isFileImport, SourceLocation _loc) :
            path(std::move(_path)), isFileImport(_isFileImport)
        {
            type     = AstType::ImportStmt;
            location = std::move(_loc);
        }

        virtual String toString() const override
        {
            return std::format("<ImportStmt '{}'{}>", path, isFileImport ? " [file]" : "");
        }
    };
} // namespace Fig
