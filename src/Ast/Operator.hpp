/*!
    @file src/Ast/Operator.hpp
    @brief 运算符定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <cstdint>

#include <Deps/Deps.hpp>
#include <Token/Token.hpp>

namespace Fig
{
    enum class UnaryOperator : std::uint8_t
    {
        BitNot,    // 位运算取反 ~
        Negate,    // 取反 -
        Not,       // 逻辑非 ! / not
        AddressOf, // 取引用 &
    };
    enum class BinaryOperator : std::uint8_t
    {
        Add,      // 加 +
        Subtract, // 减 -
        Multiply, // 乘 *
        Divide,   // 除 /
        Modulo,   // 取模 %

        Equal,        // 等于 ==
        NotEqual,     // 不等于 !=
        Less,         // 小于 <
        Greater,      // 大于 >
        LessEqual,    // 小于等于 <=
        GreaterEqual, // 大于等于 >=

        Is, // is操作符

        LogicalAnd, // 逻辑与 && / and
        LogicalOr,  // 逻辑或 || / or

        Power, // 幂运算 **

        Assign,        // 赋值(修改) =
        AddAssign,     // +=
        SubAssign,    // -=
        MultiplyAssign, // *=
        DivideAssign,    // /=
        ModuloAssign,  // %=
        BitXorAssign,    // ^=

        // 位运算
        BitAnd,     // 按位与 &
        BitOr,      // 按位或 |
        BitXor,     // 异或 ^
        ShiftLeft,  // 左移
        ShiftRight, // 右移

        // 成员访问
        MemberAccess, // .
    };

    using BindingPower = unsigned int;

    HashMap<TokenType, UnaryOperator> &GetUnaryOpMap();
    HashMap<TokenType, BinaryOperator> &GetBinaryOpMap();

    HashMap<UnaryOperator, BindingPower> &GetUnaryOpBindingPowerMap();
    HashMap<BinaryOperator, BindingPower> &GetBinaryOpBindingPowerMap();

    BindingPower GetUnaryOpRBp(UnaryOperator);

    BindingPower GetBinaryOpLBp(BinaryOperator);
    BindingPower GetBinaryOpRBp(BinaryOperator);

    bool IsTokenOp(TokenType type, bool binary = true);

    UnaryOperator TokenToUnaryOp(const Token &);
    BinaryOperator TokenToBinaryOp(const Token &);
}; // namespace Fig