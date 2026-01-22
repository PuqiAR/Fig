#pragma once

#include <Ast/functionParameters.hpp>
#include <Ast/astBase.hpp>


namespace Fig::Ast
{
    /*
    
    interface Readable
    {
        read() -> String
        {
            // default
        }
        
        flush() -> Null; // non-default
    }
    
    */


    struct InterfaceMethod
    {
        FString name;
        FunctionParameters paras;
        Expression returnType;

        BlockStatement defaultBody = nullptr; // nullptr is non-default func

        bool hasDefaultBody() const
        {
            return defaultBody != nullptr;
        }
    };

    class InterfaceDefAst final : public StatementAst
    {
    public:
        FString name;
        std::vector<InterfaceMethod> methods;
        std::vector<FString> parents; // Feature, NOT NOW
        bool isPublic;

        InterfaceDefAst()
        {
            type = AstType::InterfaceDefSt;
        }

        InterfaceDefAst(FString _name, std::vector<InterfaceMethod> _methods, bool _isPublic) :
            name(std::move(_name)), methods(std::move(_methods)), isPublic(_isPublic)
        {
            type = AstType::InterfaceDefSt;
        }
    };

    using InterfaceDef = std::shared_ptr<InterfaceDefAst>;
};