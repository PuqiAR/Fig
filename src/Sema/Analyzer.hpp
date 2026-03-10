/*!
    @file src/Sema/Analyzer.hpp
    @brief 语义分析器定义
*/

#pragma once

#include <Ast/Ast.hpp>
#include <Sema/Type.hpp>
#include <Sema/Environment.hpp>
#include <Utils/Arena.hpp>
#include <Error/Diagnostics.hpp>
#include <SourceManager/SourceManager.hpp>

namespace Fig
{
    class Analyzer
    {
    private:
        Arena          arena; 
        SourceManager &manager;
        TypeContext    typeCtx;
        Environment    env;
        Diagnostics    diag;

        HashMap<String, BaseType*> globalTypes;
        HashMap<String, Symbol*>   globalSymbols;

        bool hasInit = false;
        bool hasMain = false;

        // 核心递归查找：解决跨越函数边界的捕获问题
        Result<Symbol*, Error> resolveSymbolInternal(const String &name, const SourceLocation &loc, Scope* startScope);

        Result<Type, Error> resolveTypeExpr(TypeExpr *texpr);
        Result<void, Error> pass1(Program *prog);
        Result<void, Error> resolveTypes(Program *prog); 
        Result<void, Error> checkBodies(Program *prog);  

        Result<void, Error> analyzeStmt(Stmt *stmt);
        Result<Type, Error> analyzeExpr(Expr *expr);
        
        int addUpvalue(Scope *scope, Symbol *target, bool isLocal);

    public:
        Analyzer(SourceManager &m) : manager(m) {}

        Result<void, Error> Analyze(Program *prog);
        
        Diagnostics& GetDiagnostics() { return diag; }
        TypeContext& GetTypeContext() { return typeCtx; }
    };
}
