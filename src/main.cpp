/*
    The Fig Programming Language
    Copyright © 2025-2026 PuqiAR (im@puqiar.top). All rights reserved.

    Under MIT License, see /LICENSE for more
*/

#include <Core/Core.hpp>
#include <VM/Entry.hpp>

#include <Utils/ArgParser/ArgParser.hpp>

int main(int argc, char **argv)
{
    using namespace Fig;

    std::ostream &err = CoreIO::GetStdErr();
    std::ostream &out = CoreIO::GetStdOut();

    ArgParser::ArgumentParser argparser("Fig", "Fig Toolchain");

    argparser.AddFlag('h', "help").Help("Print the help message");
    argparser.AddFlag('v', "version").Help("Show toolchain version");
    argparser.AddFlag("license").Help("Print the license text");

    auto res = argparser.Parse(argc, argv);
    if (!res)
    {
        err << res.error().Format() << '\n';
        return 1;
    }

    auto &args = *res;

    bool showHelp    = args.HasFlag("help");
    bool showVersion = args.HasFlag("version");
    bool showLicense = args.HasFlag("license");

    if (showHelp)
    {
        out << argparser.FormatHelp() << '\n';
        return 0;
    }

    if (showVersion)
    {
        out << std::format("Fig {}, copyright (c) 2025-2026 PuqiAR, under the {} License\n",
            Core::VERSION,
            Core::LICENSE);
        out << std::format("Build time: {} [{} x{} on {}]\n",
            Core::COMPILE_TIME,
            Core::COMPILER,
            Core::ARCH,
            Core::PLATFORM);
    }
    else if (showLicense)
    {
        out << Core::LICENSE_TEXT << '\n';
    }

    auto       &positionals = args.GetPositionals();
    const auto &posSize     = positionals.size();

    if (posSize != 1 && (showHelp || showVersion || showLicense))
    {
        return 0;
    }

    if (posSize > 1)
    {
        err << "Error: Too more positionals, expect 1. Use Fig [Fig source code file (.fig)]\n";
        return 1;
    }
    else if (posSize == 0)
    {
        err << "Error: No input source file\n";
        return 1;
    }

    const String &path = positionals.front();
    Entry::RunFromPath(path);

    return 0;
}