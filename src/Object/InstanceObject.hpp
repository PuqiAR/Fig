/*!
    @file src/Object/InstanceObject.hpp
    @brief 实例(InstanceObject)定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-10
*/

#include <Object/ObjectBase.hpp>

namespace Fig
{
    struct InstanceObject final : public Object
    {
        Value fields[];
    };
}
