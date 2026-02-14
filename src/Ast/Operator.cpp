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
        static HashMap<TokenType, BinaryOperator> binaryOpMap{{TokenType::Plus, BinaryOperator::Add},
            {TokenType::Minus, BinaryOperator::Subtract},
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

            {TokenType::Pipe, BinaryOperator::BitAnd},
            {TokenType::Ampersand, BinaryOperator::BitAnd},
            {TokenType::ShiftLeft, BinaryOperator::ShiftLeft},
            {TokenType::ShiftRight, BinaryOperator::ShiftRight}};
        return binaryOpMap;
    }

    // 赋值 < 三元 < 逻辑或 < 逻辑与 < 位运算 < 比较 < 位移 < 加减 < 乘除 < 幂 < 一元

    HashMap<UnaryOperator, BindingPower> &GetUnaryOpBindingPowerMap()
    {
        static HashMap<UnaryOperator, BindingPower> unbpm{
            {UnaryOperator::BitNot, 10000},
            {UnaryOperator::Negate, 10000},
            {UnaryOperator::Not, 10000},
            {UnaryOperator::AddressOf, 10000},
        };
        return unbpm;
    }

    HashMap<BinaryOperator, BindingPower> &GetBinaryOpBindingPowerMap()
    {
        static HashMap<BinaryOperator, BindingPower> bnbpm{{BinaryOperator::Assign, 100},

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
        };
    }
    bool IsTokenOp(TokenType type, bool binary /* = true*/)
    {
        if (binary)
        {
            return GetBinaryOpMap().contains(type);
        }
        return GetUnaryOpMap().contains(type);
    }
}; // namespace Fig