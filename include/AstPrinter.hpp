#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <ast.hpp>
#include <magic_enum/magic_enum.hpp>

using namespace Fig;
using namespace Fig::Ast;
class AstPrinter
{
public:
    void print(const AstBase &node, int indent = 0)
    {
        if (!node) return;
        switch (node->getType())
        {
            case AstType::BinaryExpr:
                printBinaryExpr(std::static_pointer_cast<BinaryExprAst>(node), indent);
                break;
            case AstType::UnaryExpr:
                printUnaryExpr(std::static_pointer_cast<UnaryExprAst>(node), indent);
                break;
            case AstType::ValueExpr:
                printValueExpr(std::static_pointer_cast<ValueExprAst>(node), indent);
                break;
            case AstType::VarDefSt:
                printVarDef(std::static_pointer_cast<VarDefAst>(node), indent);
                break;
            case AstType::VarExpr:
                printVarExpr(std::static_pointer_cast<VarExprAst>(node), indent);
                break;
            case AstType::BlockStatement:
                printBlockStatement(std::static_pointer_cast<BlockStatementAst>(node), indent);
                break;
            case AstType::FunctionCall:
                printFunctionCall(std::static_pointer_cast<FunctionCallExpr>(node), indent);
                break;
            case AstType::FunctionDefSt:
                printFunctionSt(std::static_pointer_cast<FunctionDefSt>(node), indent);
                break;
            case AstType::IfSt:
                printIfSt(std::static_pointer_cast<IfSt>(node), indent);
                break;
            case AstType::TernaryExpr:
                printTernaryExpr(std::static_pointer_cast<TernaryExprAst>(node), indent);
                break;
            default:
                printIndent(indent);
                std::cout << "Unknown AST Node\n";
        }
    }

private:
    void printIndent(int indent)
    {
        std::cout << std::string(indent, ' ');
    }

    void printFString(const Fig::FString &fstr, int indent)
    {
        printIndent(indent);
        std::cout << "FString: \"";
        std::cout.write(reinterpret_cast<const char *>(fstr.data()), fstr.size());
        std::cout << "\"\n";
    }

    template <typename EnumT>
    void printEnum(const EnumT &value, int indent)
    {
        printIndent(indent);
        std::cout << "Enum: " << magic_enum::enum_name(value) << "\n";
    }

    void printBinaryExpr(const std::shared_ptr<BinaryExprAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "BinaryExpr\n";
        printEnum(node->op, indent + 2);
        printIndent(indent + 2);
        std::cout << "Left:\n";
        print(node->lexp, indent + 4);
        printIndent(indent + 2);
        std::cout << "Right:\n";
        print(node->rexp, indent + 4);
    }

    void printUnaryExpr(const std::shared_ptr<UnaryExprAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "UnaryExpr\n";
        printEnum(node->op, indent + 2);
        printIndent(indent + 2);
        std::cout << "Expr:\n";
        print(node->exp, indent + 4);
    }

    void printValueExpr(const std::shared_ptr<ValueExprAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "ValueExpr\n";
        printFString(node->val.toString(), indent + 2);
    }

    void printVarDef(const std::shared_ptr<VarDefAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "VarDef\n";
        printIndent(indent + 2);
        std::cout << "Name: ";
        printFString(node->name, 0);
        printIndent(indent + 2);
        std::cout << "Type: ";
        printFString(node->typeName, 0);
        if (node->expr)
        {
            printIndent(indent + 2);
            std::cout << "InitExpr:\n";
            print(node->expr, indent + 4);
        }
    }

    void printVarExpr(const std::shared_ptr<VarExprAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "VarExpr\n";
        printIndent(indent + 2);
        std::cout << "Name: ";
        printFString(node->name, 0);
    }

    void printBlockStatement(const std::shared_ptr<BlockStatementAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "BlockStatement\n";
        for (const auto &stmt : node->stmts)
        {
            print(stmt, indent + 2);
        }
    }

    void printFunctionCall(const std::shared_ptr<FunctionCallExpr> &node, int indent)
    {
        printIndent(indent);
        std::cout << "FunctionCall\n";
        printIndent(indent + 2);
        printIndent(indent + 2);
    }

    void printFunctionSt(const std::shared_ptr<FunctionDefSt> &node, int indent)
    {
        printIndent(indent);
        std::cout << "FunctionSt\n";
        printIndent(indent + 2);
        std::cout << "Name: ";
        printFString(node->name, 0);
        printIndent(indent + 2);
        printIndent(indent + 2);
        std::cout << "Body:\n";
        print(node->body, indent + 4);
    }

    void printIfSt(const std::shared_ptr<IfSt> &node, int indent)
    {
        printIndent(indent);
        std::cout << "IfSt\n";
        printIndent(indent + 2);
        std::cout << "Condition:\n";
        print(node->condition, indent + 4);
        printIndent(indent + 2);
    }

    void printTernaryExpr(const std::shared_ptr<TernaryExprAst> &node, int indent)
    {
        printIndent(indent);
        std::cout << "TernaryExpr\n";
        printIndent(indent + 2);
        std::cout << "Condition:\n";
        print(node->condition, indent + 4);
        printIndent(indent + 2);
        std::cout << "TrueExpr:\n";
        print(node->valueT, indent + 4);
        printIndent(indent + 2);
        std::cout << "FalseExpr:\n";
        print(node->valueF, indent + 4);
    }
};