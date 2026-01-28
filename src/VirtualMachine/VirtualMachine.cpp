#include <Evaluator/Value/value.hpp>
#include <Evaluator/Value/Type.hpp>

#include <Bytecode/Instruction.hpp>
#include <Bytecode/CompiledFunction.hpp>
#include <VirtualMachine/VirtualMachine.hpp>

namespace Fig
{
    Object VirtualMachine::Execute()
    {
        while (currentFrame->ip < currentFrame->fn.chunk.ins.size())
        {
            Instruction ins = currentFrame->fn.chunk.ins[currentFrame->ip++];

            switch (ins.code)
            {
                case OpCode::HALT: {
                    return *Object::getNullInstance();
                }
                case OpCode::RETURN: {
                    const Object &ret = pop();

                    uint64_t base = currentFrame->base;
                    popFrame();

                    if (frames.empty())
                    {
                        return ret;
                    }

                    stack.resize(base); // 清除函数的临时值
                    push(ret);
                    break;
                }
                case OpCode::LOAD_LOCAL: {
                    uint64_t operand = static_cast<uint64_t>(ins.operand);
                    // LOCAL编号都为正数

                    push(stack[currentFrame->base + operand]);
                    break;
                }

                case OpCode::LOAD_CONST: {
                    uint64_t operand = static_cast<uint64_t>(ins.operand);
                    // CONST编号都为正数

                    push(currentFrame->fn.chunk.constants[operand]);
                    break;
                }

                case OpCode::STORE_LOCAL: {
                    uint64_t operand = static_cast<uint64_t>(ins.operand);
                    // LOCAL编号都为正数

                    stack[currentFrame->base + operand] = pop();
                    break;
                }

                case OpCode::LT: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    Object result = lhs < rhs;
                    push(result);
                    break;
                }

                case OpCode::LTET: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    Object result = lhs <= rhs;
                    push(result);
                    break;
                }

                case OpCode::GT: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    Object result = lhs > rhs;
                    push(result);
                    break;
                }

                case OpCode::GTET: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    Object result = lhs >= rhs;
                    push(result);
                    break;
                }

                case OpCode::ADD: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    if (lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs.as<ValueType::IntClass>() + rhs.as<ValueType::IntClass>();
                        push(Object(result));
                        break;
                    }

                    Object result = lhs + rhs;
                    push(result);
                    break;
                }

                case OpCode::SUB: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    if (lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs.as<ValueType::IntClass>() - rhs.as<ValueType::IntClass>();
                        push(Object(result));
                        break;
                    }

                    Object result = lhs - rhs;
                    push(result);
                    break;
                }

                case OpCode::MUL: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    if (lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>())
                    {
                        ValueType::IntClass result = lhs.as<ValueType::IntClass>() * rhs.as<ValueType::IntClass>();
                        push(Object(result));
                        break;
                    }

                    Object result = lhs * rhs;
                    push(result);
                    break;
                }

                case OpCode::DIV: {
                    const Object &rhs = pop();
                    const Object &lhs = pop();

                    if (lhs.is<ValueType::IntClass>() && rhs.is<ValueType::IntClass>())
                    {
                        ValueType::DoubleClass result = (double)lhs.as<ValueType::IntClass>() / (double)rhs.as<ValueType::IntClass>();
                        push(Object(result));
                        break;
                    }

                    Object result = lhs / rhs;
                    push(result);
                    break;
                }

                case OpCode::JUMP: {
                    int64_t target = ins.operand;
                    currentFrame->ip += target;
                    break;
                }

                case OpCode::JUMP_IF_FALSE: {
                    const Object &cond = pop();

                    if (!cond.is<bool>())
                    {
                        throw RuntimeError(
                            FString(u8"Condition must be boolean!")
                        );
                    }
                    if (!cond.as<bool>())
                    {
                        // cond is falsity
                        int64_t target = ins.operand;
                        currentFrame->ip += target;
                    }
                    break;
                }

                case OpCode::CALL:
                {
                    uint16_t argCount = static_cast<uint16_t>(ins.operand); // number of max arg is UINT16_MAX
                    
                    const Object &obj = stack.back();
                    if (!obj.is<Function>())
                    {
                        throw RuntimeError(FString(std::format("{} is not callable", obj.toString().toBasicString())));
                    }

                    // const Function &fn_obj = obj.as<Function>();
                    // assert(fn_obj.isCompiled && "function must be compiled");
                    
                    CompiledFunction fn;

                    assert(stack.size() >= argCount && "stack does not have enough arguments");
                    assert(fn.slotCount >= argCount && "slotCount < argCount");

                    uint64_t base = stack.size() - 1 - argCount;

                    pop(); // pop function

                    for (int64_t i = 0; i < fn.slotCount - argCount; ++i)
                    {
                        push(*Object::getNullInstance());
                    }

                    CallFrame newFrame 
                    {
                        0,
                        base, // 参数已经加载到stack, base为第一个参数
                        fn
                    };

                    addFrame(newFrame);
                    break;
                }
            }
        }
        return *Object::getNullInstance();
    }
}; // namespace Fig