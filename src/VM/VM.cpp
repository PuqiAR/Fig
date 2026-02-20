/*!
    @file src/VM/VM.hpp
    @brief 虚拟机核心执行引擎实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <VM/VM.hpp>

#define BINARY_ARITHMETIC_OP(opCode, op)                                                                               \
    case OpCode::opCode: {                                                                                             \
        std::uint8_t b   = decodeB(inst);                                                                              \
        std::uint8_t c   = decodeC(inst);                                                                              \
        Value        lhs = registers[b];                                                                               \
        Value        rhs = registers[c];                                                                               \
        if (lhs.IsInt() && rhs.IsInt()) [[likely]]                                                                     \
        {                                                                                                              \
            registers[a] = Value::FromInt(lhs.AsInt() op rhs.AsInt());                                                 \
        }                                                                                                              \
        else if (lhs.IsDouble() && rhs.IsDouble()) [[likely]]                                                          \
        {                                                                                                              \
            registers[a] = Value::FromDouble(lhs.AsDouble() op rhs.AsDouble());                                        \
        }                                                                                                              \
        /* 隐式类型提升：Int 与 Double 混合运算 */                                                                     \
        else if (lhs.IsInt() && rhs.IsDouble()) [[likely]]                                                             \
        {                                                                                                              \
            registers[a] = Value::FromDouble(lhs.AsInt() op rhs.AsDouble());                                           \
        }                                                                                                              \
        else if (lhs.IsDouble() && rhs.IsInt()) [[likely]]                                                             \
        {                                                                                                              \
            registers[a] = Value::FromDouble(lhs.AsDouble() op rhs.AsInt());                                           \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            assert(false && "VM Runtime Error: Unsupported types for arithmetic operation");                           \
        }                                                                                                              \
        break;                                                                                                         \
    }

#define BINARY_COMPARE_OP(opCode, op)                                                                                  \
    case OpCode::opCode: {                                                                                             \
        std::uint8_t b   = decodeB(inst);                                                                              \
        std::uint8_t c   = decodeC(inst);                                                                              \
        Value        lhs = registers[b];                                                                               \
        Value        rhs = registers[c];                                                                               \
        if (lhs.IsInt() && rhs.IsInt()) [[likely]]                                                                     \
        {                                                                                                              \
            registers[a] = (lhs.AsInt() op rhs.AsInt()) ? Value::GetTrueInstance() : Value::GetFalseInstance();        \
        }                                                                                                              \
        else if (lhs.IsDouble() && rhs.IsDouble()) [[likely]]                                                          \
        {                                                                                                              \
            registers[a] = (lhs.AsDouble() op rhs.AsDouble()) ? Value::GetTrueInstance() : Value::GetFalseInstance();  \
        }                                                                                                              \
        else if (lhs.IsInt() && rhs.IsDouble()) [[likely]]                                                             \
        {                                                                                                              \
            registers[a] = (lhs.AsInt() op rhs.AsDouble()) ? Value::GetTrueInstance() : Value::GetFalseInstance();     \
        }                                                                                                              \
        else if (lhs.IsDouble() && rhs.IsInt()) [[likely]]                                                             \
        {                                                                                                              \
            registers[a] = (lhs.AsDouble() op rhs.AsInt()) ? Value::GetTrueInstance() : Value::GetFalseInstance();     \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            /* TODO: 非数字比较 */                                                                                     \
            assert(false && "VM Runtime Error: Unsupported types for comparison");                                     \
        }                                                                                                              \
        break;                                                                                                         \
    }

namespace Fig
{
    Result<Value, Error> VM::Execute(Proto *proto)
    {
        // 指令指针 (Instruction Pointer / PC) 和 常量池指针
        const Instruction *ip = proto->code.data();
        const Value       *k  = proto->constants.data();

        // 核心解释器循环 (The Dispatch Loop)
        while (true)
        {
            // 取指并递增指针
            Instruction inst = *ip++;

            // 解码 OpCode 和 A 操作数
            OpCode       op = decodeOpCode(inst);
            std::uint8_t a  = decodeA(inst);
            switch (op)
            {
                case OpCode::Exit: {
                    return Value::GetNullInstance();
                }
                case OpCode::LoadK: {
                    std::uint16_t bx = decodeBx(inst);
                    registers[a]     = k[bx]; // constants
                    break;
                }

                case OpCode::Mov: {
                    std::uint16_t bx = decodeBx(inst);
                    registers[a]     = registers[bx];
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

                case OpCode::Return: {
                    return registers[a];
                }

                default: {
                    assert(false && "VM: Unknown OpCode encountered!");
                }
            }
        }
        return Value::GetNullInstance();
    }
}; // namespace Fig