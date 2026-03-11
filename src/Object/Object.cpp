/*!
    @file src/Object/Object.hpp
    @brief 值表示实现 (NaN Boxing) 和 堆对象函数的实现
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-19
*/

#include <Object/Object.hpp>

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
            Object *obj = AsObject();
            if (!obj)
                return "<Bad Object>";

            // 物理分发, 扔掉虚函数！awa
            switch (obj->type)
            {
                case ObjectType::String: {
                    auto *strObj = static_cast<StringObject *>(obj);
                    return strObj->data;
                }
                case ObjectType::Function: {
                    auto *fnObj = static_cast<FunctionObject *>(obj);
                    return std::format("<Function: {}>", fnObj->name);
                }
                case ObjectType::Struct: {
                    auto *structObj = static_cast<StructObject *>(obj);
                    return std::format("<Struct: {}>", structObj->name);
                }
                case ObjectType::Instance: {
                    // 利用你预留的神级指针 klass 溯源
                    if (obj->klass)
                    {
                        return std::format("<Instance: {}>", obj->klass->name);
                    }
                    return "<Instance: Unknown>";
                }
                default: return "<Corrupted Object>";
            }
        }
        else
        {
            return "Unknow";
        }
    }
}; // namespace Fig