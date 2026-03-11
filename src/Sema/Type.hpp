/*!
    @file src/Sema/Type.hpp
    @brief 类型系统定义：对齐 NaN-boxing 物理布局
*/

#pragma once
#include <Deps/Deps.hpp>
#include <Error/Error.hpp>

namespace Fig
{
    enum class TypeTag : std::uint8_t
    {
        Int,
        Double,
        String,
        Bool,
        Null,
        Any,
        Function,
        Struct,
        Interface
    };

    class BaseType;

    struct Type
    {
        BaseType *base       = nullptr;
        bool      isNullable = false;

        bool operator==(const Type &other) const
        {
            return base == other.base && isNullable == other.isNullable;
        }
        bool operator!=(const Type &other) const
        {
            return !(*this == other);
        }

        bool   is(TypeTag tag) const;
        String toString() const;

        bool isAssignableTo(const Type &target) const;
    };

    class BaseType
    {
    public:
        TypeTag tag;
        String  name;
        BaseType(TypeTag t, String n) : tag(t), name(std::move(n)) {}
        virtual ~BaseType() = default;
    };

    class FuncType : public BaseType
    {
    public:
        DynArray<Type> paramTypes;
        Type retType;
        FuncType(DynArray<Type> params, Type ret) :
            BaseType(TypeTag::Function, "Function"), paramTypes(std::move(params)), retType(ret)
        {
        }
    };

    class StructType : public BaseType
    {
    public:
        struct Field
        {
            String name;
            Type   type;
            bool   isPublic;
            int    index;
        };
        DynArray<Field>                    fields;
        HashMap<String, size_t>            fieldMap;
        HashMap<String, struct FnDefStmt *> methods;

        StructType(String n) : BaseType(TypeTag::Struct, std::move(n)) {}
        void AddField(String name, Type type, bool isPublic)
        {
            size_t idx = fields.size();
            fields.push_back({name, type, isPublic, (int) idx});
            fieldMap[name] = idx;
        }
    };

    class InterfaceType : public BaseType
    {
    public:
        struct MethodSig
        {
            String         name;
            DynArray<Type> params;
            Type           retType;
        };
        HashMap<String, MethodSig> methods;
        InterfaceType(String n) : BaseType(TypeTag::Interface, std::move(n)) {}
    };

    class TypeContext
    {
    public:
        DynArray<BaseType *> allTypes;
        BaseType            *intType, *doubleType, *stringType, *boolType, *anyType, *nullType;

        TypeContext();
        ~TypeContext();
    
        Type GetBasic(TypeTag tag, bool nullable = false);
        Type CreateFuncType(DynArray<Type> params, Type ret);
    };
} // namespace Fig
