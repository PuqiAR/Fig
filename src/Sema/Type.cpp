/*!
    @file src/Sema/Type.cpp
    @brief 类型系统实现
*/

#include <Sema/Type.hpp>

namespace Fig
{
    bool Type::is(TypeTag t) const
    {
        return base && base->tag == t;
    }

    String Type::toString() const
    {
        if (!base)
            return "Unknown";
        if (base->tag == TypeTag::Function)
        {
            auto  *ft  = static_cast<FuncType *>(base);
            String sig = "func(";
            for (size_t i = 0; i < ft->paramTypes.size(); ++i)
            {
                sig += ft->paramTypes[i].toString();
                if (i < ft->paramTypes.size() - 1)
                    sig += ", ";
            }
            sig += ") -> " + ft->retType.toString();
            return sig;
        }

        String res = base->name;
        if (isNullable && base->tag != TypeTag::Null)
            res += "?";
        return res;
    }

    bool Type::isAssignableTo(const Type &target) const
    {
        if (target.is(TypeTag::Any) || this->is(TypeTag::Any))
            return true; // Any 逃逸通道
        if (this->is(TypeTag::Null) && target.isNullable)
            return true; // Null 安全赋值
        return this->base == target.base && (!this->isNullable || target.isNullable); // 严格匹配
    }

    TypeContext::TypeContext()
    {
        intType    = new BaseType(TypeTag::Int, "Int");
        doubleType = new BaseType(TypeTag::Double, "Double");
        stringType = new BaseType(TypeTag::String, "String");
        boolType   = new BaseType(TypeTag::Bool, "Bool");
        anyType    = new BaseType(TypeTag::Any, "Any");
        nullType   = new BaseType(TypeTag::Null, "Null");

        allTypes.push_back(intType);
        allTypes.push_back(doubleType);
        allTypes.push_back(stringType);
        allTypes.push_back(boolType);
        allTypes.push_back(anyType);
        allTypes.push_back(nullType);
    }

    TypeContext::~TypeContext()
    {
        for (auto t : allTypes)
            delete t;
    }

    Type TypeContext::GetBasic(TypeTag tag, bool nullable)
    {
        BaseType *b = nullptr;
        switch (tag)
        {
            case TypeTag::Int: b = intType; break;
            case TypeTag::Double: b = doubleType; break;
            case TypeTag::String: b = stringType; break;
            case TypeTag::Bool: b = boolType; break;
            case TypeTag::Any: b = anyType; break;
            case TypeTag::Null: b = nullType; break;
            default: break;
        }
        return {b, nullable};
    }

    Type TypeContext::CreateFuncType(DynArray<Type> params, Type ret)
    {
        auto *ft = new FuncType(std::move(params), ret);
        allTypes.push_back(ft);
        return Type{ft, false};
    }
} // namespace Fig
