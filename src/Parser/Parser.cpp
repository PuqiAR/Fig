/*!
    @file src/Parser/Parser.cpp
    @brief 语法分析器实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-08
*/

#include <Parser/Parser.hpp>

namespace Fig
{
    Result<Program *, Error> Parser::Parse()
    {
        Program *program = arena.Allocate<Program>();
        
        while (currentToken().type != TokenType::EndOfFile)
        {
            auto result = parseStatement();
            if (!result)
            {
                return std::unexpected(result.error());
            }
            
            Stmt *stmt = *result;
            if (stmt)
            {
                program->nodes.push_back(stmt);
            }
        }
        
        return program;
    }
}; // namespace Fig
