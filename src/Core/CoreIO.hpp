/*!
    @file src/Core/CoreIO.hpp
    @brief 标准输入输出链接定义
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <iostream>

namespace Fig::CoreIO
{
    std::ostream &GetStdOut();
    std::ostream &GetStdErr();
    std::ostream &GetStdLog();
    std::istream &GetStdCin();
};