#pragma once

#include <Token/token.hpp>
#include <Core/fig_string.hpp>

#include <format>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <unordered_set>

namespace Fig::Ast
{
    enum class AstType : uint8_t
    {
        /* Base Class */
        _AstBase,
        StatementBase,
        ExpressionBase,
        /* Expression */
        ValueExpr,
        VarExpr,
        LambdaExpr,
        UnaryExpr,
        BinaryExpr,
        TernaryExpr,

        /* Postfix */
        MemberExpr, // a.b
        IndexExpr,  // a[b]
        FunctionCall, // a()

        /* Literals */
        ListExpr,  // [1, "2", 3
        TupleExpr, // (1, 2, 3)
        MapExpr,   // {a: 1}
        InitExpr,  // struct{"123", 456}
        FunctionLiteralExpr,

        /* Statement */
        BlockStatement,
        ExpressionStmt,

        VarDefSt,
        FunctionDefSt,
        StructSt,
        InterfaceDefSt,
        ImplementSt,

        IfSt,
        ElseSt,
        ElseIfSt,

        // VarAssignSt,
        WhileSt,
        ForSt,
        ReturnSt,
        BreakSt,
        ContinueSt,

        PackageSt,
        ImportSt,

        TrySt,
        ThrowSt,
    };

    // static const std::unordered_map<AstType, FString> astTypeToString{
    //     /* Base Class */
    //     {AstType::_AstBase, FString(u8"Ast")},
    //     {AstType::StatementBase, FString(u8"Statement")},
    //     {AstType::ExpressionBase, FString(u8"Expression")},
    //     /* Expression */
    //     {AstType::ValueExpr, FString(u8"ValueExpr")},
    //     {AstType::LambdaExpr, FString(u8"LambdaExpr")},
    //     {AstType::UnaryExpr, FString(u8"UnaryExpr")},
    //     {AstType::BinaryExpr, FString(u8"BinaryExpr")},
    //     {AstType::TernaryExpr, FString(u8"TernaryExpr")},

    //     {AstType::InitExpr, FString(u8"InitExpr")},

    //     /* Statement */
    //     {AstType::BlockStatement, FString(u8"BlockStatement")},

    //     {AstType::VarDefSt, FString(u8"VarSt")},
    //     {AstType::FunctionDefSt, FString(u8"FunctionDefSt")},
    //     {AstType::StructSt, FString(u8"StructSt")},
    //     {AstType::ImplementSt, FString(u8"ImplementSt")},

    //     {AstType::IfSt, FString(u8"IfSt")},
    //     {AstType::ElseSt, FString(u8"ElseSt")},
    //     {AstType::ElseIfSt, FString(u8"ElseIfSt")},
    //     {AstType::VarAssignSt, FString(u8"VarAssignSt")},
    //     {AstType::WhileSt, FString(u8"WhileSt")},
    //     {AstType::ReturnSt, FString(u8"ReturnSt")},
    //     {AstType::BreakSt, FString(u8"BreakSt")},
    //     {AstType::ContinueSt, FString(u8"ContinueSt")},
    // };

    struct AstAddressInfo
    {
        size_t line, column;
    };

    class _AstBase
    {
    protected:
        AstType type;
        AstAddressInfo aai;

    public:
        _AstBase(const _AstBase &) = default;
        _AstBase(_AstBase &&) = default;

        _AstBase &operator=(const _AstBase &) = default;
        _AstBase &operator=(_AstBase &&) = default;

        _AstBase() {}

        void setAAI(AstAddressInfo _aai)
        {
            aai = std::move(_aai);
        }

        virtual FString typeName()
        {
            return FString::fromStringView(
                FStringView::fromBasicStringView(magic_enum::enum_name(type)));
        }
        virtual FString toString()
        {
            return FString(std::format("<Base Ast '{}' at {}:{}>", typeName().toBasicString(), aai.line, aai.column));
        }

        AstAddressInfo getAAI()
        {
            return aai;
        }

        AstType getType()
        {
            return type;
        }
    };

    class StatementAst : public _AstBase
    {
    public:
        using _AstBase::_AstBase;
        using _AstBase::operator=;
        StatementAst()
        {
            type = AstType::StatementBase;
        }

        virtual FString toString() override
        {
            return FString(std::format("<Stmt Ast '{}' at {}:{}>", typeName().toBasicString(), aai.line, aai.column));
        }
    };

    class EofStmt final : public StatementAst
    {
    public:
        EofStmt()
        {
            type = AstType::StatementBase;
        }

        virtual FString toString() override
        {
            return FString(std::format("<EOF Stmt at {}:{}>", aai.line, aai.column));
        }
    };

    class ExpressionAst : public _AstBase
    {
    public:
        using _AstBase::_AstBase;
        using _AstBase::operator=;
        ExpressionAst()
        {
            type = AstType::ExpressionBase;
        }

        virtual FString toString() override
        {
            return FString(std::format("<Expr Ast '{}' at {}:{}>", typeName().toBasicString(), aai.line, aai.column));
        }
    };
    enum class Operator : uint8_t
    {
        LeftParen,
        RightParen,

        // 算术
        Add,      // +
        Subtract, // -
        Multiply, // *
        Divide,   // /
        Modulo,   // %
        Power,    // **

        // 逻辑
        And, // and / &&
        Or,  // or / ||
        Not, // not / !

        // 比较
        Equal,        // ==
        NotEqual,     // !=
        Less,         // <
        LessEqual,    // <=
        Greater,      // >
        GreaterEqual, // >=
        Is,           // a is b

        // 三目
        TernaryCond,

        // 位运算
        BitAnd,     // &
        BitOr,      // |
        BitXor,     // ^
        BitNot,     // ~
        ShiftLeft,  // <<
        ShiftRight, // >>

        // 赋值表达式
        Assign,         // =
        PlusAssign,     // +=
        MinusAssign,    // -=
        AsteriskAssign, // *=
        SlashAssign,    // /=
        PercentAssign,  // %=
        CaretAssign,    // ^=
        // Walrus, // :=
    };

    static const std::unordered_set<Operator> unaryOps{
        Operator::Not,      // !
        Operator::Subtract, // -
        Operator::BitNot, // ~

        Operator::BitAnd, // reference operator &
    };
    static const std::unordered_set<Operator> binaryOps{
        Operator::Add,
        Operator::Subtract,
        Operator::Multiply,
        Operator::Divide,
        Operator::Modulo,
        Operator::Power,
        Operator::And,
        Operator::Or,

        Operator::Equal,
        Operator::NotEqual,
        Operator::Less,
        Operator::LessEqual,
        Operator::Greater,
        Operator::GreaterEqual,
        Operator::Is,

        Operator::BitAnd,
        Operator::BitOr,
        Operator::BitXor,
        Operator::BitNot,
        Operator::ShiftLeft,
        Operator::ShiftRight,

        Operator::Assign,
        Operator::PlusAssign,
        Operator::MinusAssign,
        Operator::AsteriskAssign,
        Operator::SlashAssign,
        Operator::CaretAssign

        // Operator::Walrus,
        // Operator::Dot
    };
    static const std::unordered_set<Operator> ternaryOps{Operator::TernaryCond};

    static const std::unordered_map<TokenType, Operator> TokenToOp{
        // 算术
        {TokenType::Plus, Operator::Add},
        {TokenType::Minus, Operator::Subtract},
        {TokenType::Asterisk, Operator::Multiply},
        {TokenType::Slash, Operator::Divide},
        {TokenType::Percent, Operator::Modulo},
        {TokenType::Power, Operator::Power},

        // 逻辑
        {TokenType::And, Operator::And},
        {TokenType::DoubleAmpersand, Operator::And},
        {TokenType::Or, Operator::Or},
        {TokenType::DoublePipe, Operator::Or},
        {TokenType::Not, Operator::Not},

        // 比较
        {TokenType::Equal, Operator::Equal},
        {TokenType::NotEqual, Operator::NotEqual},
        {TokenType::Less, Operator::Less},
        {TokenType::LessEqual, Operator::LessEqual},
        {TokenType::Greater, Operator::Greater},
        {TokenType::GreaterEqual, Operator::GreaterEqual},
        {TokenType::Is, Operator::Is},

        // 三目
        {TokenType::Question, Operator::TernaryCond},

        // 位运算
        {TokenType::Ampersand, Operator::BitAnd},
        {TokenType::Pipe, Operator::BitOr},
        {TokenType::Caret, Operator::BitXor},
        {TokenType::Tilde, Operator::BitNot},
        {TokenType::ShiftLeft, Operator::ShiftLeft},
        {TokenType::ShiftRight, Operator::ShiftRight},

        // 赋值表达式
        {TokenType::Assign, Operator::Assign},
        {TokenType::PlusEqual, Operator::PlusAssign},
        {TokenType::MinusEqual, Operator::MinusAssign},
        {TokenType::AsteriskEqual, Operator::AsteriskAssign},
        {TokenType::SlashEqual, Operator::SlashAssign},
        {TokenType::PercentEqual, Operator::PercentAssign},
        {TokenType::CaretEqual, Operator::CaretAssign},
        // {TokenType::Walrus, Operator::Walrus},
    }; // :=

    inline bool isOpUnary(Operator op)
    {
        return unaryOps.contains(op);
    }
    inline bool isOpBinary(Operator op)
    {
        return binaryOps.contains(op);
    }
    inline bool isOpTernary(Operator op)
    {
        return ternaryOps.contains(op);
    }

    using AstBase = std::shared_ptr<_AstBase>;
    using Statement = std::shared_ptr<StatementAst>;
    using Expression = std::shared_ptr<ExpressionAst>;
    using Eof = std::shared_ptr<EofStmt>;

    class BlockStatementAst : public StatementAst
    {
    public:
        std::vector<Statement> stmts;
        BlockStatementAst()
        {
            type = AstType::BlockStatement;
        }
        BlockStatementAst(std::vector<Statement> _stmts) :
            stmts(std::move(_stmts))
        {
            type = AstType::BlockStatement;
        }
        virtual FString typeName() override
        {
            return FString(u8"BlockStatement");
        }
        virtual FString toString() override
        {
            return FString(std::format("<StmtAst '{}' at {}:{}>", typeName().toBasicString(), aai.line, aai.column));
        }
        virtual ~BlockStatementAst() = default;
    };

    using BlockStatement = std::shared_ptr<BlockStatementAst>;
    // static BlockStatement builtinEmptyBlockSt(new BlockStatementAst());
}; // namespace Fig::Ast