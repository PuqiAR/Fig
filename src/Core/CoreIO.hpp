#pragma once

#include <iostream>

namespace Fig::CoreIO
{
    std::ostream &GetStdOut();
    std::ostream &GetStdErr();
    std::ostream &GetStdLog();
    std::istream &GetStdCin();
};