/*!
    @file src/VM/VM.cpp
    @brief 虚拟机核心执行引擎实现
*/

#include <Core/Core.hpp>
#include <VM/VM.hpp>

#define BINARY_ARITHMETIC_OP(opName, op)                                                           \
    do_##opName:                                                                                   \
    {                                                                                              \
        std::uint8_t a   = decodeA(inst);                                                          \
        std::uint8_t b   = decodeB(inst);                                                          \
        std::uint8_t c   = decodeC(inst);                                                          \
        Value        lhs = currentFrame->registerBase[b];                                          \
        Value        rhs = currentFrame->registerBase[c];                                          \
        if (lhs.IsInt() && rhs.IsInt()) [[likely]]                                                 \
        {                                                                                          \
            currentFrame->registerBase[a] = Value::FromInt(lhs.AsInt() op rhs.AsInt());            \
        }                                                                                          \
        else if (lhs.IsDouble() && rhs.IsDouble()) [[likely]]                                      \
        {                                                                                          \
            currentFrame->registerBase[a] = Value::FromDouble(lhs.AsDouble() op rhs.AsDouble());   \
        }                                                                                          \
        else if (lhs.IsInt() && rhs.IsDouble()) [[likely]]                                         \
        {                                                                                          \
            currentFrame->registerBase[a] = Value::FromDouble(lhs.AsInt() op rhs.AsDouble());      \
        }                                                                                          \
        else if (lhs.IsDouble() && rhs.IsInt()) [[likely]]                                         \
        {                                                                                          \
            currentFrame->registerBase[a] = Value::FromDouble(lhs.AsDouble() op rhs.AsInt());      \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            assert(false && "VM Runtime Error: Unsupported types for arithmetic operation");       \
        }                                                                                          \
        DISPATCH();                                                                                \
    }

#define BINARY_COMPARE_OP(opName, op)                                                              \
    do_##opName:                                                                                   \
    {                                                                                              \
        std::uint8_t a   = decodeA(inst);                                                          \
        std::uint8_t b   = decodeB(inst);                                                          \
        std::uint8_t c   = decodeC(inst);                                                          \
        Value        lhs = currentFrame->registerBase[b];                                          \
        Value        rhs = currentFrame->registerBase[c];                                          \
        if (lhs.IsInt() && rhs.IsInt()) [[likely]]                                                 \
        {                                                                                          \
            currentFrame->registerBase[a] = (lhs.AsInt() op rhs.AsInt()) ?                         \
                                                Value::GetTrueInstance() :                         \
                                                Value::GetFalseInstance();                         \
        }                                                                                          \
        else if (lhs.IsDouble() && rhs.IsDouble()) [[likely]]                                      \
        {                                                                                          \
            currentFrame->registerBase[a] = (lhs.AsDouble() op rhs.AsDouble()) ?                   \
                                                Value::GetTrueInstance() :                         \
                                                Value::GetFalseInstance();                         \
        }                                                                                          \
        else if (lhs.IsInt() && rhs.IsDouble()) [[likely]]                                         \
        {                                                                                          \
            currentFrame->registerBase[a] = (lhs.AsInt() op rhs.AsDouble()) ?                      \
                                                Value::GetTrueInstance() :                         \
                                                Value::GetFalseInstance();                         \
        }                                                                                          \
        else if (lhs.IsDouble() && rhs.IsInt()) [[likely]]                                         \
        {                                                                                          \
            currentFrame->registerBase[a] = (lhs.AsDouble() op rhs.AsInt()) ?                      \
                                                Value::GetTrueInstance() :                         \
                                                Value::GetFalseInstance();                         \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            assert(false && "VM Runtime Error: Unsupported types for comparison");                 \
        }                                                                                          \
        DISPATCH();                                                                                \
    }

namespace Fig
{
    Result<Value, Error> VM::Execute(CompiledModule *compiledModule)
    {
        Proto *entry = compiledModule->protos[0];
        (void) pushFrame(entry, registers); // 刚开始执行寄存器不会溢出

        // 对齐 Bytecode.hpp 中的 OpCode 顺序
        static const void *dispatchTable[] = {&&do_Exit,
            &&do_Exit_MaxRecursionDepthExceeded,

            &&do_LoadK,
            &&do_LoadTrue,
            &&do_LoadFalse,
            &&do_LoadNull,

            &&do_FastCall,
            &&do_Call,
            &&do_Return,

            &&do_LoadFn,

            &&do_Jmp,
            &&do_JmpIfFalse,

            &&do_Mov,

            &&do_Add,
            &&do_Sub,
            &&do_Mul,
            &&do_Div,
            &&do_Mod,
            &&do_BitXor,

            &&do_IntFastAdd,
            &&do_IntFastSub,
            &&do_IntFastMul,
            &&do_IntFastDiv,

            &&do_Equal,
            &&do_NotEqual,
            &&do_Greater,
            &&do_Less,
            &&do_GreaterEqual,
            &&do_LessEqual,

            &&do_GetGlobal,
            &&do_SetGlobal,
            &&do_GetUpval,
            &&do_SetUpval,
            &&do_Copy,

            &&do_Count};

        Instruction inst;

#define DISPATCH()                                                                                 \
    do                                                                                             \
    {                                                                                              \
        inst = *(currentFrame->ip++);                                                              \
        goto *dispatchTable[inst & 0xFF];                                                          \
    } while (0)

        // 引擎点火!! :3
        DISPATCH();

    do_Exit: {
        return Value::FromInt(decodeSBx(inst));
    }

    do_Exit_MaxRecursionDepthExceeded: {
        CoreIO::GetStdErr() << std::format(
            "Oops! max recursion depth limit {} exceeded in Fn `{}` , exiting...\n",
            MAX_RECURSION_DEPTH,
            (currentFrame - 1)->proto->name); // pushFrame失败了，但currentFrame仍然移动，所以
                                              // (currentFrame - 1)是 lastFrame
        std::exit(static_cast<int>(MAX_RECURSION_DEPTH));
    }

    do_LoadK: {
        std::uint8_t  a               = decodeA(inst);
        std::uint16_t bx              = decodeBx(inst);
        currentFrame->registerBase[a] = currentFrame->getConstant(bx);
        DISPATCH();
    }

    do_LoadTrue: {
        std::uint8_t a                = decodeA(inst);
        currentFrame->registerBase[a] = Value::GetTrueInstance();
        DISPATCH();
    }

    do_LoadFalse: {
        std::uint8_t a                = decodeA(inst);
        currentFrame->registerBase[a] = Value::GetFalseInstance();
        DISPATCH();
    }

    do_LoadNull: {
        std::uint8_t a                = decodeA(inst);
        currentFrame->registerBase[a] = Value::GetNullInstance();
        DISPATCH();
    }

    do_FastCall: {
        std::uint8_t a       = decodeA(inst);
        Proto       *proto   = compiledModule->protos[a];
        std::uint8_t baseReg = decodeB(inst);

        currentFrame->ip = pushFrame(proto, currentFrame->registerBase + baseReg);

        DISPATCH();
    }

    do_Call: {
        // TODO: FunctionObject 动态解包

        std::uint8_t a       = decodeA(inst);
        std::uint8_t baseReg = decodeB(inst);

        Value callee = currentFrame->registerBase[a];

        FunctionObject *closure = nullptr;

        if (!callee.IsObject())
        {
            size_t ipIdx = currentFrame->ip - currentFrame->proto->code.data();

            return std::unexpected(Error(ErrorType::TypeError,
                std::format("Object `{}` is not callable", callee.ToString()),
                "none",
                *currentFrame->proto->locations[ipIdx]));
        }
        else
        {
            Object *obj = callee.AsObject();
            if (!obj->isFunction())
            {
                size_t ipIdx = currentFrame->ip - currentFrame->proto->code.data();

                return std::unexpected(Error(ErrorType::TypeError,
                    std::format("Object `{}` is not callable", callee.ToString()),
                    "none",
                    *currentFrame->proto->locations[ipIdx]));
            }
            closure = static_cast<FunctionObject *>(obj);
        }

        currentFrame->ip = pushFrame(closure, currentFrame->registerBase + baseReg);

        DISPATCH();
    }

    do_Return: {
        std::uint8_t a      = decodeA(inst);
        Value        retVal = currentFrame->registerBase[a];

        closeUpvalues(currentFrame->registerBase);

        // 此时 registerBase[0] 指向的是 Caller 的 baseReg 槽位
        currentFrame->registerBase[0] = retVal;
        popFrame();

        DISPATCH();
    }

    do_LoadFn: {
        std::uint8_t  a  = decodeA(inst);
        std::uint16_t bx = decodeBx(inst);

        Proto *p = compiledModule->protos[bx];

        size_t upValSize = p->upvalues.size();
        size_t extraSize = upValSize * sizeof(Upvalue *);

        FunctionObject *closure =
            (FunctionObject *) allocateObject<FunctionObject>(ObjectType::Function, extraSize);

        // CoreIO::GetStdErr() << "DEBUG: p->name = " << p->name << '\n';
        new (&closure->name) String(p->name); // String非平凡类型，有自己的构造函数
        closure->proto        = p;
        closure->paraCount    = p->numParams;
        closure->upvalueCount = static_cast<std::uint32_t>(upValSize);

        for (size_t i = 0; i < closure->upvalueCount; ++i)
        {
            auto &info = p->upvalues[i];
            if (info.isLocal)
            {
                Value   *targetSlot = &currentFrame->registerBase[info.index];
                Upvalue *prev       = nullptr;
                Upvalue *curr       = openUpvalues;

                while (curr != nullptr && curr->location > targetSlot)
                {
                    prev = curr;
                    curr = curr->next;
                }

                if (curr != nullptr && curr->location == targetSlot)
                {
                    // 如果别的闭包已经捕获了这个槽位，共享物理指针
                    closure->upvalues[i] = curr;
                    ++curr->refCount;
                }
                else
                {
                    // 首次捕获
                    Upvalue *uv = new Upvalue;

                    uv->location = targetSlot;
                    uv->next     = curr;
                    ++uv->refCount;

                    if (prev == nullptr)
                        openUpvalues = uv;
                    else
                        prev->next = uv;
                    closure->upvalues[i] = uv;
                }
            }
            else
            {
                closure->upvalues[i] = currentFrame->closure->upvalues[info.index];
            }
        }

        currentFrame->registerBase[a] = Value::FromObject(closure);
        DISPATCH();
    }

    do_Jmp: {
        std::int16_t sbx = decodeSBx(inst);
        currentFrame->ip += sbx;
        DISPATCH();
    }

    do_JmpIfFalse: {
        std::uint8_t a = decodeA(inst);
        Value       &v = currentFrame->registerBase[a];
        if (!v.AsBool())
        {
            std::int16_t sbx = decodeSBx(inst);
            currentFrame->ip += sbx;
        }
        DISPATCH();
    }

    do_Mov: {
        std::uint8_t  a               = decodeA(inst);
        std::uint16_t bx              = decodeBx(inst);
        currentFrame->registerBase[a] = currentFrame->registerBase[bx];
        DISPATCH();
    }

        BINARY_ARITHMETIC_OP(Add, +);
        BINARY_ARITHMETIC_OP(Sub, -);
        BINARY_ARITHMETIC_OP(Mul, *);
        BINARY_ARITHMETIC_OP(Div, /);

    do_Mod:
    do_BitXor:
        assert(false && "VM: Mod and BitXor not fully implemented yet!");
        DISPATCH();

    do_IntFastAdd: {
        std::uint8_t a = decodeA(inst);
        std::uint8_t b = decodeB(inst);
        std::uint8_t c = decodeC(inst);

        Value l = currentFrame->registerBase[b];
        Value r = currentFrame->registerBase[c];

        currentFrame->registerBase[a] = Value::FromInt(l.AsInt() + r.AsInt());
        DISPATCH();
    }

    do_IntFastSub: {
        std::uint8_t a = decodeA(inst);
        std::uint8_t b = decodeB(inst);
        std::uint8_t c = decodeC(inst);

        Value l = currentFrame->registerBase[b];
        Value r = currentFrame->registerBase[c];

        currentFrame->registerBase[a] = Value::FromInt(l.AsInt() - r.AsInt());
        DISPATCH();
    }

    do_IntFastMul: {
        std::uint8_t a = decodeA(inst);
        std::uint8_t b = decodeB(inst);
        std::uint8_t c = decodeC(inst);

        Value l = currentFrame->registerBase[b];
        Value r = currentFrame->registerBase[c];

        currentFrame->registerBase[a] = Value::FromInt(l.AsInt() * r.AsInt());
        DISPATCH();
    }

    do_IntFastDiv: {
        std::uint8_t a = decodeA(inst);
        std::uint8_t b = decodeB(inst);
        std::uint8_t c = decodeC(inst);

        Value l = currentFrame->registerBase[b];
        Value r = currentFrame->registerBase[c];

        currentFrame->registerBase[a] =
            Value::FromDouble(static_cast<double>(l.AsInt()) / r.AsInt());
        DISPATCH();
    }

        BINARY_COMPARE_OP(Equal, ==);
        BINARY_COMPARE_OP(NotEqual, !=);
        BINARY_COMPARE_OP(Greater, >);
        BINARY_COMPARE_OP(Less, <);
        BINARY_COMPARE_OP(GreaterEqual, >=);
        BINARY_COMPARE_OP(LessEqual, <=);

    do_GetGlobal: {
        std::uint8_t  a               = decodeA(inst);
        std::uint16_t bx              = decodeBx(inst);
        currentFrame->registerBase[a] = globals[bx];
        DISPATCH();
    }

    do_SetGlobal: {
        std::uint8_t  a  = decodeA(inst);
        std::uint16_t bx = decodeBx(inst);
        globals[bx]      = currentFrame->registerBase[a];
        DISPATCH();
    }

    do_GetUpval: {
        std::uint8_t a = decodeA(inst);
        std::uint8_t b = decodeB(inst);

        currentFrame->registerBase[a] = *(currentFrame->closure->upvalues[b]->location);
        DISPATCH();
    }

    do_SetUpval: {
        std::uint8_t a                                  = decodeA(inst);
        std::uint8_t b                                  = decodeB(inst);
        *(currentFrame->closure->upvalues[b]->location) = currentFrame->registerBase[a]; // copy
        DISPATCH();
    }

    do_Copy: {
        std::uint8_t a                = decodeA(inst);
        std::uint8_t b                = decodeB(inst);
        currentFrame->registerBase[a] = currentFrame->registerBase[b];
        DISPATCH();
    }

    do_Count: {
        assert(false && "Hit Count sentinel!");
        return Value::GetNullInstance();
    }
    }
}; // namespace Fig
