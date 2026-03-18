/*!
    @file src/VM/Entry.hpp
    @brief vm入口定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-03-13
*/

#include <Deps/Deps.hpp>

namespace Fig::Entry
{
    struct Config
    {
        enum Mode
        {
            Debug,
            Normal
        } mode;
        bool dump;
    };
    void RunFromPath(const String &, const Config &conf);
};