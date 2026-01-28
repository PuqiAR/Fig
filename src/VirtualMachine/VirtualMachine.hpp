#pragma once

#include <Evaluator/Value/value.hpp>
#include <Bytecode/CallFrame.hpp>

#include <vector>

namespace Fig
{
    class VirtualMachine
    {
    private:
        std::vector<CallFrame> frames;
        CallFrame *currentFrame;

        std::vector<Object> stack;
        Object *stack_top;

    public:
        void Clean()
        {
            frames.clear();
            stack.clear();

            currentFrame = nullptr;
            stack_top = nullptr;
        }

        void addFrame(const CallFrame &_frame)
        {
            frames.push_back(_frame);
            currentFrame = &frames.back();
        }

        CallFrame popFrame()
        {
            assert((!frames.empty()) && "frames is empty!");

            CallFrame back = *currentFrame;
            frames.pop_back();
            currentFrame = (frames.empty() ? nullptr : &frames.back());

            return back;
        }

        void push(const Object &_object)
        {
            stack.push_back(_object);
            stack_top = &stack.back();
        }

        void nextIns(CallFrame &frame) const
        {
            frame.ip += 1;
        }

        Object pop()
        {
            assert((!stack.empty()) && "stack is empty!");

            Object back = *stack_top;
            stack.pop_back();
            stack_top = (stack.empty() ? nullptr : &stack.back());

            return back;
        }

        VirtualMachine(const CallFrame &_frame) 
        {
            addFrame(_frame);
        }

        Object Execute();
    };
};