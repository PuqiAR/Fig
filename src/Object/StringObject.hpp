/*!
    @file src/Object/StringObject.hpp
    @brief 字符串对象标识
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

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
    struct StringObject final : public Object
    {
        String data; // 40 bytes

        virtual String toString() const override
        {
            return data;
        }
    };
};