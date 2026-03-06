/*!
    @file src/VM/VM.hpp
    @brief 虚拟机核心执行引擎实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <VM/VM.hpp>

#define BINARY_ARITHMETIC_OP(opCode, op)                                                           \
    case OpCode::opCode: {                                                                         \
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
        /* 隐式类型提升：Int 与 Double 混合运算 */                                                 \
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
        break;                                                                                     \
    }

#define BINARY_COMPARE_OP(opCode, op)                                                              \
    case OpCode::opCode: {                                                                         \
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
            /* TODO: 非数字比较 */                                                                 \
            assert(false && "VM Runtime Error: Unsupported types for comparison");                 \
        }                                                                                          \
        break;                                                                                     \
    }

namespace Fig
{
    Result<Value, Error> VM::Execute(CompiledModule *compiledModule)
    {
        Proto *entry = compiledModule->protos[0];
        pushFrame(entry, registers);

        while (true)
        {
            // 取指并递增指针
            Instruction inst = *(currentFrame->ip++);

            // 解码 OpCode 和 A 操作数
            OpCode       op = decodeOpCode(inst);
            std::uint8_t a  = decodeA(inst);
            switch (op)
            {
                case OpCode::Exit: { [[unlikely]]
                    return Value::GetNullInstance();
                }

                case OpCode::LoadK: {
                    std::uint16_t bx = decodeBx(inst);
                    currentFrame->registerBase[a] = currentFrame->getConstant(bx); // constants
                    break;
                }

                case OpCode::LoadTrue: {
                    currentFrame->registerBase[a] = Value::GetTrueInstance();
                    break;
                }

                case OpCode::LoadFalse: {
                    currentFrame->registerBase[a] = Value::GetFalseInstance();
                    break;
                }

                case OpCode::LoadNull: {
                    currentFrame->registerBase[a] = Value::GetNullInstance();
                    break;
                }

                case OpCode::FastCall: {
                    Proto *proto = compiledModule->protos[a];
                    std::uint8_t baseReg = decodeB(inst);

                    pushFrame(proto, currentFrame->registerBase + baseReg);
                    break;
                }

                case OpCode::Call: {
                    break;
                }

                case OpCode::Return: {
                    *currentFrame->registerBase = currentFrame->registerBase[a];
                    popFrame();
                    break;
                }

                case OpCode::Jmp: {
                    std::int16_t sbx = decodeSBx(inst);
                    currentFrame->ip += sbx;
                    break;
                }

                case OpCode::JmpIfFalse: {
                    Value &v = currentFrame->registerBase[a];
                    bool cond = v.AsBool(); // 条件类型 Compiler检查
                    if (!cond)
                    {
                        std::int16_t sbx = decodeSBx(inst);
                        currentFrame->ip += sbx;
                    }
                    break;
                }

                case OpCode::Mov: {
                    std::uint16_t bx = decodeBx(inst);
                    currentFrame->registerBase[a] = currentFrame->registerBase[bx];
                    break;
                }

                BINARY_ARITHMETIC_OP(Add, +);
                BINARY_ARITHMETIC_OP(Sub, -);
                BINARY_ARITHMETIC_OP(Mul, *);
                BINARY_ARITHMETIC_OP(Div, /);
                
                BINARY_COMPARE_OP(Equal, ==);
                BINARY_COMPARE_OP(NotEqual, !=);
                BINARY_COMPARE_OP(Greater, >);
                BINARY_COMPARE_OP(Less, <);
                BINARY_COMPARE_OP(GreaterEqual, >=);
                BINARY_COMPARE_OP(LessEqual, <=);


                // default: {
                //     assert(false && "VM: Unknown OpCode encountered!");
                // }
            }
        }
        return Value::GetNullInstance();
    }
}; // namespace Fig