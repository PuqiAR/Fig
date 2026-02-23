/*!
    @file src/Parser/Parser.cpp
    @brief 语法分析器(Pratt + 手动递归下降) 实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/


#include <Parser/Parser.hpp>

namespace Fig
{
    Result<Program *, Error> Parser::Parse()
    {
        Program *program = new Program;
        while (!isEOF)
        {
            const auto &result = parseStatement();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            Stmt *stmt = *result;
            if (!stmt)
            {
                continue;
            }
            program->nodes.push_back(stmt);
        }
        return program;
    }
};