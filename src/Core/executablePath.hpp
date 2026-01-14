#pragma once

#include <filesystem>
#include <libloaderapi.h>

#ifdef _WIN32
    #include <libloaderapi.h>
#endif

namespace Fig
{
    inline std::filesystem::path getExecutablePath()
    {
#ifdef _WIN32
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        return std::filesystem::path(buffer);
#else
        return std::filesystem::canonical("/proc/self/exe");
#endif
    }
}; // namespace Fig