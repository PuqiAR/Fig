/*!
    @file src/Object/Object.hpp
    @brief 值表示实现 (NaN Boxing) 和 堆对象函数的实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Object/ObjectBase.hpp>

namespace Fig
{
    String Value::ToString() const
    {
        if (IsNull())
        {
            return "null";
        }
        else if (IsInt())
        {
            return std::to_string(AsInt());
        }
        else if (IsDouble())
        {
            return std::format("{}", AsDouble());
        }
        else if (IsBool())
        {
            return (AsBool() ? "true" : "false");
        }
        else if (IsObject())
        {
            return "Object"; // TODO: 分派
        }
        else
        {
            return "Unknow";
        }
    }
}; // namespace Fig