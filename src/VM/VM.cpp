/*!
    @file src/VM/VM.cpp
    @brief 虚拟机核心执行引擎实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <VM/VM.hpp>

// Computed GOTO!!!
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
        pushFrame(entry, registers);

        // 🔥 必须与 Bytecode.hpp 中的 OpCode 枚举严格一一对应！
        static const void *dispatchTable[] = {&&do_Exit,
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
            &&do_Equal,
            &&do_NotEqual,
            &&do_Greater,
            &&do_Less,
            &&do_GreaterEqual,
            &&do_LessEqual,
            &&do_Count};

        Instruction inst;

// 🔥 核心分发引擎：取指 -> 直接查表并 Jump
#define DISPATCH()                                                                                 \
    do                                                                                             \
    {                                                                                              \
        inst = *(currentFrame->ip++);                                                              \
        goto *dispatchTable[inst & 0xFF];                                                          \
    } while (0)

        // 引擎点火！
        DISPATCH();

    do_Exit: {
        [[unlikely]] return Value::GetNullInstance();
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
        pushFrame(proto, currentFrame->registerBase + baseReg);
        DISPATCH();
    }

    do_Call: {
        // TODO: FunctionObject 动态解包
        DISPATCH();
    }

    do_Return: {
        std::uint8_t a              = decodeA(inst);
        *currentFrame->registerBase = currentFrame->registerBase[a];
        popFrame();
        DISPATCH();
    }

    do_LoadFn: {
        // std::uint8_t a = decodeA(inst);
        // std::uint16_t bx = decodeBx(inst);
        // TODO: R[a] = new FunctionObject(compiledModule->protos[bx])
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

        BINARY_COMPARE_OP(Equal, ==);
        BINARY_COMPARE_OP(NotEqual, !=);
        BINARY_COMPARE_OP(Greater, >);
        BINARY_COMPARE_OP(Less, <);
        BINARY_COMPARE_OP(GreaterEqual, >=);
        BINARY_COMPARE_OP(LessEqual, <=);

    do_Count: {
        assert(false && "Hit Count sentinel!");
        return Value::GetNullInstance();
    }
    }
}; // namespace Fig