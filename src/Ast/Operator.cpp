/*!
    @file src/Ast/Operator.cpp
    @brief 运算符定义内函数实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <Ast/Operator.hpp>

namespace Fig
{
    HashMap<TokenType, UnaryOperator> &GetUnaryOpMap()
    {
        static HashMap<TokenType, UnaryOperator> unaryOpMap{
            {TokenType::Tilde, UnaryOperator::BitNot},
            {TokenType::Minus, UnaryOperator::Negate},
            {TokenType::Not, UnaryOperator::Not},
            {TokenType::Ampersand, UnaryOperator::AddressOf},
        };
        return unaryOpMap;
    }

    HashMap<TokenType, BinaryOperator> &GetBinaryOpMap()
    {
        static HashMap<TokenType, BinaryOperator> binaryOpMap{
            {TokenType::Plus, BinaryOperator::Add},
            {TokenType::Minus, BinaryOperator::Subtract},
            {TokenType::Asterisk, BinaryOperator::Multiply},
            {TokenType::Slash, BinaryOperator::Divide},
            {TokenType::Percent, BinaryOperator::Modulo},

            {TokenType::Equal, BinaryOperator::Equal},
            {TokenType::NotEqual, BinaryOperator::NotEqual},
            {TokenType::Less, BinaryOperator::Less},
            {TokenType::Greater, BinaryOperator::Greater},
            {TokenType::LessEqual, BinaryOperator::LessEqual},
            {TokenType::GreaterEqual, BinaryOperator::GreaterEqual},

            {TokenType::Is, BinaryOperator::Is},

            {TokenType::And, BinaryOperator::LogicalAnd},
            {TokenType::Or, BinaryOperator::LogicalOr},

            {TokenType::Power, BinaryOperator::Power},

            {TokenType::Assign, BinaryOperator::Assign},
            {TokenType::PlusEqual, BinaryOperator::AddAssign},
            {TokenType::MinusEqual, BinaryOperator::SubAssign},
            {TokenType::AsteriskEqual, BinaryOperator::MultiplyAssign},
            {TokenType::SlashEqual, BinaryOperator::DivideAssign},
            {TokenType::PercentEqual, BinaryOperator::ModuloAssign},
            {TokenType::CaretEqual, BinaryOperator::BitXorAssign},

            {TokenType::Pipe, BinaryOperator::BitAnd},
            {TokenType::Ampersand, BinaryOperator::BitAnd},
            {TokenType::ShiftLeft, BinaryOperator::ShiftLeft},
            {TokenType::ShiftRight, BinaryOperator::ShiftRight},

            {TokenType::Dot, BinaryOperator::MemberAccess},
        };
        return binaryOpMap;
    }

    // 赋值 < 三元 < 逻辑或 < 逻辑与 < 位运算 < 比较 < 位移 < 加减 < 乘除 < 幂 < 一元 < 成员访问 < (后缀)

    /*
        暂划分:
            二元运算符：0 - 20000
            一元运算符：20001 - 40000
            后缀/成员/其他：40001 - 60001

    */

    HashMap<UnaryOperator, BindingPower> &GetUnaryOpBindingPowerMap()
    {
        static HashMap<UnaryOperator, BindingPower> unbpm{
            {UnaryOperator::BitNot, 20001},
            {UnaryOperator::Negate, 20001},
            {UnaryOperator::Not, 20001},
            {UnaryOperator::AddressOf, 20001},
        };
        return unbpm;
    }

    HashMap<BinaryOperator, BindingPower> &GetBinaryOpBindingPowerMap()
    {
        static HashMap<BinaryOperator, BindingPower> bnbpm{
            {BinaryOperator::Assign, 100},
            {BinaryOperator::AddAssign, 100},
            {BinaryOperator::SubAssign, 100},
            {BinaryOperator::MultiplyAssign, 100},
            {BinaryOperator::DivideAssign, 100},
            {BinaryOperator::ModuloAssign, 100},
            {BinaryOperator::BitXorAssign, 100},

            {BinaryOperator::LogicalOr, 500},
            {BinaryOperator::LogicalAnd, 550},

            {BinaryOperator::BitOr, 1000},
            {BinaryOperator::BitXor, 1100},
            {BinaryOperator::BitAnd, 1200},

            {BinaryOperator::Equal, 2000},
            {BinaryOperator::NotEqual, 2000},

            {BinaryOperator::Less, 2100},
            {BinaryOperator::LessEqual, 2100},
            {BinaryOperator::Greater, 2100},
            {BinaryOperator::GreaterEqual, 2100},

            {BinaryOperator::Is, 2100},

            {BinaryOperator::ShiftLeft, 3000},
            {BinaryOperator::ShiftRight, 3000},

            {BinaryOperator::Add, 4000},
            {BinaryOperator::Subtract, 4000},
            {BinaryOperator::Multiply, 4500},
            {BinaryOperator::Divide, 4500},

            {BinaryOperator::Power, 5000},

            {BinaryOperator::MemberAccess, 40001},
        };
        return bnbpm;
    }

    BindingPower GetUnaryOpRBp(UnaryOperator op)
    {
        return GetUnaryOpBindingPowerMap().at(op);
    }

    BindingPower GetBinaryOpLBp(BinaryOperator op)
    {
        return GetBinaryOpBindingPowerMap().at(op);
    }

    BindingPower GetBinaryOpRBp(BinaryOperator op)
    {
        /*
            右结合，左绑定力 >= 右
            a = b = c
            a = (b = c)
            a.b.c
        */
        switch (op)
        {
            case BinaryOperator::Assign: return GetBinaryOpLBp(op);
            case BinaryOperator::AddAssign: return GetBinaryOpLBp(op);
            case BinaryOperator::SubAssign: return GetBinaryOpLBp(op);
            case BinaryOperator::MultiplyAssign: return GetBinaryOpLBp(op);
            case BinaryOperator::DivideAssign: return GetBinaryOpLBp(op);
            case BinaryOperator::ModuloAssign: return GetBinaryOpLBp(op);
            case BinaryOperator::BitXorAssign: return GetBinaryOpLBp(op);
            case BinaryOperator::Power: return GetBinaryOpLBp(op);

            default:
                /*
                    左结合, 左绑定力 < 右
                    a * b * c
                    (a * b) * c
                */
                return GetBinaryOpLBp(op) + 1;
        }
    }

    bool IsTokenOp(TokenType type, bool binary /* = true*/)
    {
        if (binary)
        {
            return GetBinaryOpMap().contains(type);
        }
        return GetUnaryOpMap().contains(type);
    }

    UnaryOperator TokenToUnaryOp(const Token &token)
    {
        return GetUnaryOpMap().at(token.type);
    }
    BinaryOperator TokenToBinaryOp(const Token &token)
    {
        return GetBinaryOpMap().at(token.type);
    }

}; // namespace Fig