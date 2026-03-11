/*!
    @file src/Object/FunctionObject.hpp
    @brief 函数对象定义
*/

#pragma once

#include <Object/ObjectBase.hpp>
#include <Bytecode/Bytecode.hpp>

namespace Fig
{
    // Upvalue (Stack Open / Heap Closed)
    struct Upvalue
    {
        Value   *location; // Open 状态指向 VM 的 registerBase[x]，Closed 状态指向下面的 closedValue
        Value    closedValue;       // 栈帧销毁时，数据物理迁移至此
        Upvalue *next;              // 侵入式链表，供 VM 追踪当前 Open 的 Upvalue
        std::uint32_t refCount = 0; // 多少个闭包正在使用
    };

    struct FunctionObject final : public Object
    {
        String        name;
        Proto        *proto; // 静态只读字节码
        std::uint8_t  paraCount;
        std::uint32_t upvalueCount; // 捕获数量

        // 柔性数组
        Upvalue *upvalues[];

        FunctionObject(const String &_name, Proto *_proto, std::uint32_t _upvalueCount) :
            name(_name), proto(_proto), paraCount(_proto->numParams), upvalueCount(_upvalueCount)
        {
            type = ObjectType::Function;
        }

        ~FunctionObject() = default;
    };
} // namespace Fig