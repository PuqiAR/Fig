#include <Value/Type.hpp>
#include <Value/value.hpp>
#include <Context/context.hpp>

// #include <iostream>

namespace Fig
{
    std::map<FString, size_t> TypeInfo::typeMap = {};

    TypeInfo::TypeInfo() : // only allow use in evaluate time !! <---- dynamic type system requirement
        id(1), name(FString(u8"Any"))
    {
    }
    TypeInfo::TypeInfo(const FString &_name, bool reg)
    {
        static size_t id_count = 0;
        name = _name;
        // std::cerr << "TypeInfo constructor called for type name: " << name.toBasicString() << "\n";
        if (reg)
        {
            typeMap[name] = ++id_count;
            id = id_count;
        }
        else
        {
            if (!typeMap.contains(_name))
            {
                throw RuntimeError(FString(std::format(
                    "No type named '{}'",
                    _name.toBasicString())));
                // *this = ValueType::String;
            }
            id = typeMap.at(name); // may throw
        }
    }

    size_t ValueKeyHash::operator()(const ValueKey &key) const
    {
        {
            ObjectPtr value = key.value;
            const TypeInfo &type = value->getTypeInfo();

            if (type == ValueType::Int)
            {
                return std::hash<ValueType::IntClass>{}(value->as<ValueType::IntClass>());
            }
            if (type == ValueType::Double)
            {
                return std::hash<ValueType::DoubleClass>{}(value->as<ValueType::DoubleClass>());
            }
            if (type == ValueType::String)
            {
                return std::hash<ValueType::StringClass>{}(value->as<ValueType::StringClass>());
            }
            if (type == ValueType::Bool)
            {
                return std::hash<ValueType::BoolClass>{}(value->as<ValueType::BoolClass>());
            }
            if (type == ValueType::StructType)
            {
                auto HashFields = [](std::vector<Field> fields) {
                    size_t r = 0;
                    for (auto &f : fields)
                    {
                        r += std::hash<Field>{}(f);
                    }
                    return r;
                };
                const StructType &st = value->as<StructType>();
                return std::hash<TypeInfo>{}(st.type) + HashFields(st.fields);
            }
            if (type == ValueType::StructInstance)
            {
                const StructInstance &si = value->as<StructInstance>();
                return std::hash<TypeInfo>{}(si.parentType) + std::hash<uint64_t>{}(reinterpret_cast<uint64_t>(std::addressof(*si.localContext)));
            }
            assert(false);
            throw ""; // ignore warning
        }
    }

    FString prettyType(std::shared_ptr<const Object> obj)
    {
        auto t = obj->getTypeInfo();
        if (t == ValueType::StructInstance)
            return obj->as<StructInstance>().parentType.toString();
        return t.toString();
    }

    const TypeInfo ValueType::Any(FString(u8"Any"), true);                       // id: 1
    const TypeInfo ValueType::Null(FString(u8"Null"), true);                     // id: 2
    const TypeInfo ValueType::Int(FString(u8"Int"), true);                       // id: 3
    const TypeInfo ValueType::String(FString(u8"String"), true);                 // id: 4
    const TypeInfo ValueType::Bool(FString(u8"Bool"), true);                     // id: 5
    const TypeInfo ValueType::Double(FString(u8"Double"), true);                 // id: 6
    const TypeInfo ValueType::Function(FString(u8"Function"), true);             // id: 7
    const TypeInfo ValueType::StructType(FString(u8"StructType"), true);         // id: 8
    const TypeInfo ValueType::StructInstance(FString(u8"StructInstance"), true); // id: 9
    const TypeInfo ValueType::List(FString(u8"List"), true);                     // id: 10
    const TypeInfo ValueType::Map(FString(u8"Map"), true);                       // id: 11
    const TypeInfo ValueType::Module(FString(u8"Module"), true);                 // id: 12
    const TypeInfo ValueType::InterfaceType(FString(u8"InterfaceType"), true);   // id: 13



    bool implements(const TypeInfo &structType, const TypeInfo &interfaceType, ContextPtr ctx)
    {
        return ctx->hasImplRegisted(structType, interfaceType);
    }

    bool isTypeMatch(const TypeInfo &expected, ObjectPtr obj, ContextPtr ctx)
    {
        if (expected == ValueType::Any)
            return true;

        TypeInfo actual = obj->getTypeInfo();

        if (obj->is<StructInstance>())
        {
            const StructInstance &si = obj->as<StructInstance>();
            if (si.parentType == expected)
            {
                return true;
            }
            if (implements(si.parentType, expected, ctx))
            {
                return true;
            }
            return false;
        }
        else
        {
            return expected == actual;
        }
    }

} // namespace Fig