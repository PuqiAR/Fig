#pragma once

#include <Core/fig_string.hpp>
#include <cstdint>
#include <string_view>

#define __FCORE_VERSION "0.3.4-alpha"

#if defined(_WIN32)
    #define __FCORE_PLATFORM "Windows"
#elif defined(__APPLE__)
    #define __FCORE_PLATFORM "Apple"
#elif defined(__linux__)
    #define __FCORE_PLATFORM "Linux"
#elif defined(__unix__)
    #define __FCORE_PLATFORM "Unix"
#else
    #define __FCORE_PLATFORM "Unknown"
#endif

#if defined(__GNUC__)
    #if defined(_WIN32)
        #if defined(__clang__)
            #define __FCORE_COMPILER "llvm-mingw"
        #else
            #define __FCORE_COMPILER "MinGW"
        #endif

    #else
        #define __FCORE_COMPILER "GCC"
    #endif
#elif defined(__clang__)
    #define __FCORE_COMPILER "Clang"
#elif defined(_MSC_VER)
    #define __FCORE_COMPILER "MSVC"
#else
    #define __FCORE_COMPILER "Unknown"
#endif

#if SIZE_MAX == 18446744073709551615ull
    #define __FCORE_ARCH "64"
#else
    #define __FCORE_ARCH "86"
#endif

namespace Fig
{
    namespace Core
    {
        inline constexpr std::string_view VERSION = __FCORE_VERSION;
        inline constexpr std::string_view LICENSE = "MIT";
        inline constexpr std::string_view AUTHOR = "PuqiAR";
        inline constexpr std::string_view PLATFORM = __FCORE_PLATFORM;
        inline constexpr std::string_view COMPILER = __FCORE_COMPILER;
        inline constexpr std::string_view COMPILE_TIME = __FCORE_COMPILE_TIME;
        inline constexpr std::string_view ARCH = __FCORE_ARCH;
        inline constexpr FString MAIN_FUNCTION = u8"main";
    }; // namespace Core
}; // namespace Fig