#pragma once

#include <Deps/Deps.hpp>

namespace Fig
{
    struct SourcePosition
    {
        size_t line, column, tok_length;

        SourcePosition() { line = column = tok_length = 0; }
        SourcePosition(size_t _line, size_t _column, size_t _tok_length)
        {
            line = _line;
            column = _column;
            tok_length = _tok_length;
        }
    };

    struct SourceLocation
    {
        SourcePosition sp;

        Deps::String fileName;
        Deps::String packageName;
        Deps::String functionName;

        SourceLocation() {}
        SourceLocation(SourcePosition _sp,
                       Deps::String _fileName,
                       Deps::String _packageName,
                       Deps::String _functionName)
        {
            sp = std::move(_sp);
            fileName = std::move(_fileName);
            packageName = std::move(_packageName);
            functionName = std::move(_functionName);
        }
        SourceLocation(size_t line,
                       size_t column,
                       size_t tok_length,
                       Deps::String _fileName,
                       Deps::String _packageName,
                       Deps::String _functionName)
        {
            sp = SourcePosition(line, column, tok_length);
            fileName = std::move(_fileName);
            packageName = std::move(_packageName);
            functionName = std::move(_functionName);
        }
    };
}; // namespace Fig