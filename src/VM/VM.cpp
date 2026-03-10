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
            (currentFrame - 1)->proto->name); // pushFrame失败了，但currentFrame仍然移动，所以 (currentFrame - 1)是 lastFrame
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
        DISPATCH();
    }

    do_Return: {
        std::uint8_t a      = decodeA(inst);
        Value        retVal = currentFrame->registerBase[a];

        // 此时 registerBase[0] 指向的是 Caller 的 baseReg 槽位

        currentFrame->registerBase[0] = retVal;
        popFrame();

        DISPATCH();
    }

    do_LoadFn: {
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
        assert(false && "VM: GetUpval requires FunctionObject (Closure) implementation");
        DISPATCH();
    }

    do_SetUpval: {
        assert(false && "VM: SetUpval requires FunctionObject (Closure) implementation");
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
