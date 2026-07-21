/*!
    @file src/Object/StructObject.hpp
    @brief 结构体类型 StructObject 定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <Ast/Operator.hpp>
#include <Object/ObjectBase.hpp>
#include <Sema/Type.hpp>

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
    struct FieldMeta
    {
        String name;
        Type   type;
    };

    struct StructObject final : public Object
    {
        String          name;
        std::uint8_t    fieldCount;
        FieldMeta      *fields;   // [fieldCount]
        Object         *operators[GetOperatorsSize()];
        // operators: [UnaryOp 0..N][BinaryOp 0..N], nullptr = 无重载

        Object *GetUnaryOperator(UnaryOperator _op)
        {
            return operators[static_cast<std::uint8_t>(_op)];
        }

        Object *GetBinaryOperator(BinaryOperator _op)
        {
            return operators[static_cast<std::uint8_t>(UnaryOperator::Count)
                             + static_cast<std::uint8_t>(_op)];
        }
    };
}; // namespace Fig