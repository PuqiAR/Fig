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
            MapElement
        } kind;
        std::shared_ptr<VariableSlot> slot;

        ObjectPtr listOrMap = nullptr;
        size_t listIndex;

        ObjectPtr mapIndex;

        LvObject(std::shared_ptr<VariableSlot> _slot) :
            slot(std::move(_slot))
        {
            kind = Kind::Variable;
        }
        LvObject(ObjectPtr _v, size_t _index) :
            listOrMap(_v), listIndex(_index)
        {
            assert(_v->getTypeInfo() == ValueType::List);
            kind = Kind::ListElement;
        }
        LvObject(ObjectPtr _v, ObjectPtr _index) :
            listOrMap(_v), mapIndex(_index)
        {
            assert(_v->getTypeInfo() == ValueType::Map);
            kind = Kind::MapElement;
        }

        const ObjectPtr &get() const
        {
            if (kind == Kind::Variable)
            {
                auto s = resolve(slot);
                return s->value;
            }
            else if (kind == Kind::ListElement)
            {
                List &list = listOrMap->as<List>();
                if (listIndex >= list.size())
                    throw RuntimeError(FString(
                        std::format("Index {} out of range", listIndex)));
                return list.at(listIndex);
            }
            else // map
            {
                Map &map = listOrMap->as<Map>();
                if (!map.contains(mapIndex))
                    throw RuntimeError(FString(
                        std::format("Key {} not found", mapIndex->toString().toBasicString())));
                return map.at(mapIndex);
            }
        }

        void set(const ObjectPtr &v)
        {
            if (kind == Kind::Variable)
            {
                auto s = resolve(slot);
                if (s->declaredType != ValueType::Any && s->declaredType != v->getTypeInfo())
                {
                    throw RuntimeError(
                        FString(
                            std::format("Variable `{}` expects type `{}`, but got '{}'",
                                        s->name.toBasicString(),
                                        s->declaredType.toString().toBasicString(),
                                        v->getTypeInfo().toString().toBasicString())));
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
                List &list = listOrMap->as<List>();
                if (listIndex >= list.size())
                    throw RuntimeError(FString(
                        std::format("Index {} out of range", listIndex)));
                list[listIndex] = v;
            }
            else // map
            {
                Map &map = listOrMap->as<Map>();
                map[mapIndex] = v;
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
}