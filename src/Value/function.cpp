#include <value.hpp>
#include <Value/function.hpp>

namespace Fig
{
    FunctionStruct::FunctionStruct(std::function<Value(const std::vector<Value> &)> fn,
                                   int argc) :
        id(nextId()),
        isBuiltin(true),
        builtin(std::move(fn)),
        builtinParamCount(argc) {}
        
    Function::Function(std::function<Value(const std::vector<Value> &)> fn,
                       int argc) :
        __ValueWrapper(ValueType::Function)
    {
        data = std::make_unique<FunctionStruct>(std::move(fn), argc);
    }
}; // namespace Fig