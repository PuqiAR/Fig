/*!
    @file src/Object/FunctionObject.hpp
    @brief 函数对象定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-28
*/

#pragma once

#include <Object/ObjectBase.hpp>


namespace Fig
{
    // 运行时闭包对象 (24字节 Base + 8字节 Proto指针 = 32 bytes)

    struct Proto;
    struct FunctionObject final : public Object
    {
        Proto *proto; // 指向编译器生成的只读字节码与常量池

        // TODO: 实现闭包时 加一个 Upvalue 指针数组
        // Value* upvalues;
    };
} // namespace Fig