/*!
    @file src/Sema/Type.hpp
    @brief 前端类型检查的类型定义和类型驻留池
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-23
*/

#pragma once

#include <Deps/Deps.hpp>
#include <cstdint>

namespace Fig
{
    enum class TypeTag : std::uint8_t
    {
        Any,  // 动态类型底线
        Null, // 空值
        Int,
        Double,
        Bool,
        String,
        Function,
        Struct,
    };

    struct TypeInfo
    {
        TypeTag tag;
        String  name; // 完整路径序列化, 如 Int, std.file.File

        bool isAny() const
        {
            return tag == TypeTag::Any;
        }
    };

    // 全局唯一类型驻留池
    class TypeContext
    {
    private:
        DynArray<TypeInfo *> allTypes;

        // 缓存
        TypeInfo *typeAny;
        TypeInfo *typeNull;
        TypeInfo *typeInt;
        TypeInfo *typeDouble;
        TypeInfo *typeBool;
        TypeInfo *typeString;
        TypeInfo *typeFunction;
        TypeInfo *typeStruct;

    public:
        TypeInfo *GetAny()
        {
            return typeAny;
        }
        TypeInfo *GetNull()
        {
            return typeNull;
        }
        TypeInfo *GetInt()
        {
            return typeInt;
        }
        TypeInfo *GetDouble()
        {
            return typeDouble;
        }
        TypeInfo *GetBool()
        {
            return typeBool;
        }
        TypeInfo *GetString()
        {
            return typeString;
        }
        TypeInfo *GetFunction()
        {
            return typeFunction;
        }
        TypeInfo *GetStruct()
        {
            return typeStruct;
        }

        TypeInfo *ResolveTypePath(const String &fullName)
        {
            for (auto *t : allTypes)
            {
                if (t->name == fullName)
                    return t;
            }
            return nullptr; // 没找到该类型
        }

        TypeInfo *ResolveTypePath(const DynArray<String> &path)
        {
            // TODO: 支持Module 系统, 查 Module 的导出表
            String fullName = path.empty() ? "" : path[0];
            for (size_t i = 1; i < path.size(); ++i)
            {
                fullName += "." + path[i];
            }

            return ResolveTypePath(fullName);
        }

        ~TypeContext()
        {
            for (TypeInfo *t : allTypes)
                delete t;
        }

        TypeContext()
        {
            typeAny      = createBuiltin(TypeTag::Any, "Any");
            typeNull     = createBuiltin(TypeTag::Null, "Null");
            typeInt      = createBuiltin(TypeTag::Int, "Int");
            typeDouble   = createBuiltin(TypeTag::Double, "Double");
            typeBool     = createBuiltin(TypeTag::Bool, "Bool");
            typeString   = createBuiltin(TypeTag::String, "String");
            typeFunction = createBuiltin(TypeTag::Function, "Function");
            typeStruct   = createBuiltin(TypeTag::Struct, "Struct");
        }

    private:
        TypeInfo *createBuiltin(TypeTag tag, String name)
        {
            TypeInfo *t = new TypeInfo{tag, std::move(name)};
            allTypes.push_back(t);
            return t;
        }
    };
}; // namespace Fig