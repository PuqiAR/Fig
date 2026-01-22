#pragma once

#include <Core/fig_string.hpp>
#include <Evaluator/Value/value.hpp>

namespace Fig
{
    struct StatementResult
    {
        ObjectPtr result;
        enum class Flow
        {
            Normal,
            Return,
            Break,
            Continue,
            Error
        } flow;

        StatementResult(ObjectPtr val, Flow f = Flow::Normal) : result(val), flow(f) {}

        static StatementResult normal(ObjectPtr val = Object::getNullInstance())
        {
            return StatementResult(val, Flow::Normal);
        }
        static StatementResult returnFlow(ObjectPtr val) { return StatementResult(val, Flow::Return); }
        static StatementResult breakFlow() { return StatementResult(Object::getNullInstance(), Flow::Break); }
        static StatementResult continueFlow() { return StatementResult(Object::getNullInstance(), Flow::Continue); }
        static StatementResult errorFlow(ObjectPtr val) { return StatementResult(val, Flow::Error); }

        bool isNormal() const { return flow == Flow::Normal; }
        bool shouldReturn() const { return flow == Flow::Return; }
        bool shouldBreak() const { return flow == Flow::Break; }
        bool shouldContinue() const { return flow == Flow::Continue; }
        bool isError() const { return flow == Flow::Error; }
    };
};