#pragma once

#include <Core/CoreInfos.hpp>
#include <Deps/HashMap/HashMap.hpp>
#include <Deps/String/String.hpp>

namespace Fig
{
    #ifdef __FCORE_LINK_DEPS
        using Deps::String;
        using Deps::HashMap;
    #endif
};