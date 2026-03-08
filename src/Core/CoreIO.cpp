/*!
    @file src/Core/CoreIO.cpp
    @brief 标准输入输出链接
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#include <Core/CoreIO.hpp>
#include <Core/CoreInfos.hpp>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace Fig::CoreIO
{
#if defined(_WIN32) || defined(__APPLE__) || defined (__linux__) || defined (__unix__)
    std::ostream &GetStdOut()
    {
        static std::ostream &out = std::cout;
        return out;
    }
    std::ostream &GetStdErr()
    {
        static std::ostream &err = std::cerr;
        return err;
    }
    std::ostream &GetStdLog()
    {
        static std::ostream &log = std::clog;
        return log;
    }
    std::istream &GetStdCin()
    {
        static std::istream &cin = std::cin;
        return cin;
    }
#else
    // link
#endif

    void InitConsoleIO()
    {
#ifdef _WIN32
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
#endif
    }
};