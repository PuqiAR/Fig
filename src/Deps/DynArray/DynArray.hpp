/*!
    @file src/Deps/DynArray/DynArray
    @brief 依赖库DynArray定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <vector>

namespace Fig::Deps
{
    template<class _Tp, class _Allocator = std::allocator<_Tp>>
    using DynArray = std::vector<_Tp, _Allocator>;
};