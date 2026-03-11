/*!
    @file src/VM/VM.hpp
    @brief 虚拟机核心执行引擎
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#pragma once

#include <Compiler/Compiler.hpp>
#include <Object/Object.hpp>
#include <Core/Core.hpp>

#include <cassert>
#include <iostream> // debug
#include <print>

namespace Fig
{
    struct CallFrame
    {
        FunctionObject *closure;      // 动态闭包Context (FastCall时 null)
        Proto          *proto;        // 当前执行的原型
        Instruction    *ip;           // 当前指令指针
        Value          *registerBase; // 寄存器起点

        inline Value getConstant(std::uint16_t idx)
        {
            return proto->constants[idx];
        }
    };

    enum class GCPhase : std::uint8_t
    {
        Idle,
        MarkRoots,
        Marking,
        Sweeping
    };

    class VM
    {
    private:
        static constexpr unsigned int MAX_REGISTERS       = 1024;
        static constexpr unsigned int MAX_GLOBALS         = 65536;
        static constexpr unsigned int MAX_RECURSION_DEPTH = 2233;

        Instruction POISON_MAX_RECURSION_DEPTH_EXCEED_INST;

        // 一次性分配
        Value registers[MAX_REGISTERS];
        Value globals[MAX_GLOBALS];

        CallFrame  frames[MAX_RECURSION_DEPTH];
        CallFrame *currentFrame;
        CallFrame *frameLimit;

        Upvalue *openUpvalues = nullptr;

        // GC
        Object            *objects = nullptr; // 链表头
        DynArray<Object *> grayStack;

        size_t  allocatedBytes = 0;
        size_t  nextGC         = 1024 * 1024; // byte, 1MB初始阈值
        GCPhase gcPhase        = GCPhase::Idle;

    private:
        inline void closeUpvalues(Value *level)
        {
            // 函数销毁时，逃逸即将销毁的变量
            while (openUpvalues && openUpvalues->location >= level)
            {
                Upvalue *upval     = openUpvalues;
                upval->closedValue = *upval->location;
                upval->location    = &upval->closedValue;
                openUpvalues       = upval->next;
            }
        }

        template <typename T>
        [[nodiscard]] T *allocateObject(ObjectType type, size_t extraBytes)
        {

            if (allocatedBytes > nextGC) // 超出阈值
            {
                switch (gcPhase)
                {
                    case GCPhase::Idle: markRoots(); break;
                    case GCPhase::Marking: stepMarking(); break;
                    case GCPhase::Sweeping: sweep(); break;
                }
            }

            size_t totalSize = sizeof(T) + extraBytes;

            // 物理分配
            T *obj = static_cast<T *>(std::malloc(totalSize));
            if (!obj) [[unlikely]]
            {
                // 分配失败
                CoreIO::GetStdErr() << "Oops! Object allocating failed! Exiting...\n";
                std::exit(1);
            }

            // 构造 Header
            obj->type  = type;
            obj->color = GCColor::Black;
            obj->klass = nullptr;
            obj->next  = objects; // 插入全局追踪链表
            objects    = obj;

            allocatedBytes += totalSize;
            return obj;
        }

        inline void writeBarrier(Object *parent, Value childVal)
        {
            if (!childVal.IsObject())
                return; // 栈对象无需

            Object *child = childVal.AsObject();
            // 三色不变式, 黑色对象绝对不能指向白色对象
            if (parent->color == GCColor::Black && child->color == GCColor::White)
            {
                child->color = GCColor::Gray;
                grayStack.push_back(child);
            }
        }

        inline void markValue(Value value)
        {
            if (!value.IsObject())
                return;

            Object *obj = value.AsObject();
            if (obj && obj->color == GCColor::White)
            {
                obj->color = GCColor::Gray;
                grayStack.push_back(obj);
            }
        }

        void markRoots()
        {
            // 扫描全局变量
            for (std::uint32_t i = 0; i < MAX_GLOBALS; ++i)
            {
                markValue(globals[i]);
            }

            // 扫描vm全部栈 [registers[0], currentFrame]
            if (currentFrame && currentFrame->proto)
            {
                Value *stackTop = currentFrame->registerBase + currentFrame->proto->maxRegisters;
                for (Value *slot = registers; slot < stackTop; ++slot)
                {
                    markValue(*slot);
                }
            }

            // 扫描逃逸链表 (Open Upvalues)
            for (Upvalue *uv = openUpvalues; uv != nullptr; uv = uv->next)
            {
                markValue(uv->closedValue);
            }

            gcPhase = GCPhase::Marking;
        }

        void stepMarking()
        {
            // 每次步进处理的对象数量
            constexpr int WORK_LIMIT = 64;
            int           workCount  = 0;

            while (!grayStack.empty() && workCount++ < WORK_LIMIT)
            {
                Object *obj = grayStack.back();
                grayStack.pop_back();

                // 标记为黑色：表示该对象及其子引用已处理完毕
                obj->color = GCColor::Black;

                switch (obj->type)
                {
                    case ObjectType::Function: {
                        auto *fn = static_cast<FunctionObject *>(obj);
                        for (std::uint32_t i = 0; i < fn->upvalueCount; ++i)
                        {
                            if (fn->upvalues[i])
                                markValue(*(fn->upvalues[i]->location));
                        }
                        break;
                    }
                    case ObjectType::Instance: {
                        auto *inst = static_cast<InstanceObject *>(obj);
                        if (inst->klass)
                            markValue(Value::FromObject(inst->klass));
                        // 扫描所有实例字段
                        std::uint8_t fieldCount = inst->klass ? inst->klass->fieldCount : 0;
                        for (std::uint8_t i = 0; i < fieldCount; ++i)
                        {
                            markValue(inst->fields[i]);
                        }
                        break;
                    }
                    case ObjectType::Struct: {
                        auto *st = static_cast<StructObject *>(obj);
                        for (int i = 0; i < GetOperatorsSize(); ++i)
                        {
                            if (st->operators[i])
                                markValue(Value::FromObject(st->operators[i]));
                        }
                        break;
                    }
                    case ObjectType::String: break; // 叶子节点
                }
            }

            if (grayStack.empty())
                gcPhase = GCPhase::Sweeping;
        }

        void sweep()
        {
            Object **curr      = &objects;
            size_t   liveBytes = 0;

            while (*curr != nullptr)
            {
                Object *obj = *curr;
                if (obj->color == GCColor::White)
                {
                    *curr = obj->next;

                    // 函数 upvalue需要手动析构
                    if (obj->type == ObjectType::Function)
                    {
                        auto *fn = static_cast<FunctionObject *>(obj);
                        fn->name.~String();
                        for (std::uint32_t i = 0; i < fn->upvalueCount; ++i)
                        {
                            Upvalue *uv = fn->upvalues[i];
                            if (uv)
                            {
                                uv->refCount--; // 减引用
                                if (uv->refCount == 0)
                                {
                                    // 只有当所有闭包都死后，才 free 这个 Upvalue 结构体
                                    
                                    std::free(uv);
                                }
                            }
                        }
                    }

                    // 持有 C++ 堆资源的成员手动析构!
                    else if (obj->type == ObjectType::String)
                    {
                        static_cast<StringObject *>(obj)->data.~String();
                    }

                    std::free(obj);
                }
                else
                {
                    // 洗白，仍是一条好汉
                    // 以备下次 GC，统计存活大小
                    obj->color = GCColor::White;

                    // 计算存活对象的真实物理大小 (Header + 柔性数组/额外数据)
                    size_t objectSize = 0;
                    switch (obj->type)
                    {
                        case ObjectType::String: objectSize = sizeof(StringObject); break;
                        case ObjectType::Instance:
                            objectSize =
                                sizeof(InstanceObject)
                                + (obj->klass ? obj->klass->fieldCount * sizeof(Value) : 0);
                            break;
                        case ObjectType::Function:
                            objectSize = sizeof(FunctionObject)
                                         + (static_cast<FunctionObject *>(obj)->upvalueCount
                                             * sizeof(Upvalue *));
                            break;
                        case ObjectType::Struct: objectSize = sizeof(StructObject); break;
                    }

                    liveBytes += objectSize;

                    curr = &obj->next;
                }
            }

            allocatedBytes = liveBytes;
            // 阈值调整, 当下一次分配超过存活内存的 2 倍时触发 GC
            nextGC = (liveBytes < 512 * 1024) ? 1024 * 1024 : liveBytes * 2;

            gcPhase = GCPhase::Idle;
        }

    public:
        VM()
        {
            for (unsigned int i = 0; i < MAX_REGISTERS; ++i)
            {
                registers[i] = Value::GetNullInstance();
            }
            for (unsigned int i = 0; i < MAX_GLOBALS; ++i)
            {
                globals[i] = Value::GetNullInstance();
            }

            currentFrame = frames;
            frameLimit   = frames + MAX_RECURSION_DEPTH - 1;

            POISON_MAX_RECURSION_DEPTH_EXCEED_INST =
                Op::iAsBx(OpCode::Exit_MaxRecursionDepthExceeded, 0, 0);
        }

    private:
        [[nodiscard]]
        inline Instruction *pushFrame(Proto *proto, Value *base) // fastcall
        {
            if (++currentFrame >= frameLimit) [[unlikely]] // 达到最大递归层数
            {
                POISON_MAX_RECURSION_DEPTH_EXCEED_INST =
                    Op::iAsBx(OpCode::Exit_MaxRecursionDepthExceeded, 0, 0);
                return &POISON_MAX_RECURSION_DEPTH_EXCEED_INST;
            }

            *currentFrame = CallFrame{nullptr, proto, proto->code.data(), base};
            return currentFrame->ip;
        }

        [[nodiscard]]
        inline Instruction *pushFrame(FunctionObject *closure, Value *base) // 普通调用
        {
            if (++currentFrame >= frameLimit) [[unlikely]]
            {
                POISON_MAX_RECURSION_DEPTH_EXCEED_INST =
                    Op::iAsBx(OpCode::Exit_MaxRecursionDepthExceeded, 0, 0);
                return &POISON_MAX_RECURSION_DEPTH_EXCEED_INST;
            }

            *currentFrame = CallFrame{closure, closure->proto, closure->proto->code.data(), base};
            return currentFrame->ip;
        }

        inline void popFrame()
        {
            --currentFrame;
        }

        inline OpCode decodeOpCode(Instruction inst)
        {
            return static_cast<OpCode>(inst & 0xFF);
        }
        inline std::uint8_t decodeA(Instruction inst)
        {
            return (inst >> 8) & 0xFF;
        }
        inline std::uint16_t decodeBx(Instruction inst)
        {
            return (inst >> 16) & 0xFFFF;
        }
        inline std::uint8_t decodeB(Instruction inst)
        {
            return (inst >> 16) & 0xFF;
        }
        inline std::uint8_t decodeC(Instruction inst)
        {
            return (inst >> 24) & 0xFF;
        }
        inline std::int16_t decodeSBx(Instruction inst)
        {
            return static_cast<std::int16_t>(inst >> 16);
        }

    public:
        // 执行入口：接收 Proto
        Result<Value, Error> Execute(CompiledModule *);

        void PrintRegisters(std::ostream &ostream = CoreIO::GetStdOut())
        {
            ostream << "=== Registers ===\n";
            for (unsigned int i = 0; i < MAX_REGISTERS; ++i)
            {
                Value &v = registers[i];
                if (!v.IsNull())
                {
                    ostream << std::format("[{}] {}\n", i, v.ToString());
                }
            }
        }

        void PrintGlobals(std::ostream &ostream = CoreIO::GetStdOut()) 
        {
            ostream << "== Globals ===\n";
            for (unsigned int i = 0; i < MAX_GLOBALS; ++i)
            {
                Value &v = globals[i];
                if (!v.IsNull())
                {
                    ostream << std::format("[{}] {}\n", i, v.ToString());
                }
            }
        }
    };
} // namespace Fig