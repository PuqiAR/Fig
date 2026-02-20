/*!
    @file src/Object/Struct.hpp
    @brief 结构体类型 Struct定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <Ast/Operator.hpp>
#include <Object/ObjectBase.hpp>

namespace Fig
{
    /*
        // Total 24 bytes size
        struct Object
        {
            Object    *next;             // 8 bytes: gc链表
            Struct    *klass;            // 8 bytes: 一切皆对象，父类指针
            ObjectType type;             // 1 byte : 类型
            bool       isMarked = false; // 1 byte : gc标记
            // + 6 bytes padding
        };
    */
    struct StructObject final : public Object
    {
        String name; // 元信息(仅供调试/打印/反射)

        // 内存布局信息
        std::uint8_t fieldCount;
        Object      *operators[GetOperatorsSize()];
        /*
            运算符重载，nullptr代表无重载
            一般为 NativeFunction / Function

            排列：
                [unary              operators   ]( binary                 operators]
                0     -             UnaryOperators::Count     BinaryOperators::Count
        */

        Object *GetUnaryOperator(UnaryOperator _op)
        {
            std::uint8_t idx = static_cast<std::uint8_t>(_op);
            return operators[idx];
        }

        Object *GetBinaryOperator(BinaryOperator _op)
        {
            std::uint16_t idx = static_cast<std::uint8_t>(UnaryOperator::Count) + static_cast<std::uint8_t>(_op);
            return operators[idx];
        }
    };
}; // namespace Fig