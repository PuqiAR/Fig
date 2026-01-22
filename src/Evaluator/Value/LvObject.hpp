#pragma once

#include <Value/VariableSlot.hpp>
#include <Value/value.hpp>

namespace Fig
{

    struct LvObject
    {
        enum class Kind
        {
            Variable,
            ListElement,
            MapElement,
            StringElement,
        } kind;
        std::shared_ptr<VariableSlot> slot;

        ObjectPtr value = nullptr;
        size_t numIndex;

        ObjectPtr mapIndex;

        ContextPtr ctx;

        LvObject(std::shared_ptr<VariableSlot> _slot, ContextPtr _ctx) :
            slot(std::move(_slot)), ctx(_ctx)
        {
            kind = Kind::Variable;
        }
        LvObject(ObjectPtr _v, size_t _index, Kind _kind, ContextPtr _ctx) :
            value(_v), numIndex(_index), ctx(_ctx)
        {
            kind = _kind;
        }
        LvObject(ObjectPtr _v, ObjectPtr _index, Kind _kind, ContextPtr _ctx) :
            value(_v), mapIndex(_index), ctx(_ctx)
        {
            kind = _kind;
        }

        ObjectPtr get() const
        {
            if (kind == Kind::Variable)
            {
                auto s = resolve(slot);
                return s->value;
            }
            else if (kind == Kind::ListElement)
            {
                List &list = value->as<List>();
                if (numIndex >= list.size())
                    throw RuntimeError(FString(
                        std::format("Index {} out of range {}", numIndex, value->toString().toBasicString())));
                return list.at(numIndex).value;
            }
            else if (kind == Kind::MapElement) // map
            {
                Map &map = value->as<Map>();
                if (!map.contains(mapIndex))
                    throw RuntimeError(FString(
                        std::format("Key {} not found", mapIndex->toString().toBasicString())));
                return map.at(mapIndex);
            }
            else
            {
                // string
                FString &string = value->as<ValueType::StringClass>();
                if (numIndex >= string.length())
                    throw RuntimeError(FString(
                        std::format("Index {} out of range {}", numIndex, value->toString().toBasicString())));

                return std::make_shared<Object>(string.getRealChar(numIndex));
            }
        }

        void set(const ObjectPtr &v)
        {
            if (kind == Kind::Variable)
            {
                auto s = resolve(slot);
                if (!isTypeMatch(s->declaredType, v, ctx))
                {
                    throw RuntimeError(
                        FString(
                            std::format("Variable `{}` expects type `{}`, but got '{}'",
                                        s->name.toBasicString(),
                                        s->declaredType.toString().toBasicString(),
                                        prettyType(v).toBasicString())));
                }
                if (isAccessConst(s->am))
                {
                    throw RuntimeError(FString(
                        std::format("Variable `{}` is immutable", s->name.toBasicString())));
                }
                s->value = v;
            }
            else if (kind == Kind::ListElement)
            {
                List &list = value->as<List>();
                if (numIndex >= list.size())
                    throw RuntimeError(FString(
                        std::format("Index {} out of range", numIndex)));
                list[numIndex] = v;
            }
            else if (kind == Kind::MapElement) // map
            {
                Map &map = value->as<Map>();
                map[mapIndex] = v;
            }
            else if (kind == Kind::StringElement)
            {
                FString &string = value->as<ValueType::StringClass>();
                if (numIndex >= string.length())
                    throw RuntimeError(FString(
                        std::format("Index {} out of range {}", numIndex, value->toString().toBasicString())));
                
                if (v->getTypeInfo() != ValueType::String)
                    throw RuntimeError(FString(
                        std::format("Could not assign {} to sub string", v->toString().toBasicString())
                    ));
                const FString &strReplace = v->as<ValueType::StringClass>();
                if (strReplace.length() > 1)
                    throw RuntimeError(FString(
                        std::format("Could not assign {} to sub string, expects length 1", v->toString().toBasicString())
                ));
                string.realReplace(numIndex, strReplace);
            }
        }

        FString name() const { return resolve(slot)->name; }
        TypeInfo declaredType() const { return resolve(slot)->declaredType; }
        AccessModifier access() const { return resolve(slot)->am; }

    private:
        std::shared_ptr<VariableSlot> resolve(std::shared_ptr<VariableSlot> s) const
        {
            while (s->isRef) s = s->refTarget;
            return s;
        }
    };
} // namespace Fig