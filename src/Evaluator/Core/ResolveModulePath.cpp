#include <Core/executablePath.hpp>

#include <Evaluator/evaluator.hpp>
#include <Evaluator/evaluator_error.hpp>

namespace Fig
{

    std::filesystem::path Evaluator::resolveModulePath(const std::vector<FString> &pathVec)
    {
        namespace fs = std::filesystem;

        static const std::vector<fs::path> defaultLibraryPath{"Library", "Library/fpm"};

        std::vector<fs::path> pathToFind(defaultLibraryPath);

        fs::path interpreterPath = getExecutablePath().parent_path();

        for (fs::path &p : pathToFind)
        {
            p = interpreterPath / p; // 相对路径 -> 绝对路径
        }

        pathToFind.insert(
            pathToFind.begin(),
            fs::path(this->sourcePath.toBasicString()).parent_path()); // first search module at the source file path

        fs::path path;

        /*
        Example:
            import comp.config;
        */

        const FString &modPathStrTop = pathVec.at(0);
        fs::path modPath;

        bool found = false;
        for (auto &parentFolder : pathToFind)
        {
            modPath = parentFolder / FString(modPathStrTop + u8".fig").toBasicString();
            if (fs::exists(modPath))
            {
                path = modPath;
                found = true;
                break;
            }
            else
            {
                modPath = parentFolder / modPathStrTop.toBasicString();
                if (fs::is_directory(modPath)) // comp is a directory
                {
                    modPath = modPath / FString(modPathStrTop + u8".fig").toBasicString();
                    /*
                        if module name is a directory, we require [module
                       name].fig at the directory
                    */
                    if (!fs::exists(modPath))
                    {
                        throw RuntimeError(FString(std::format("requires module file, {}\\{}",
                                                               modPathStrTop.toBasicString(),
                                                               FString(modPathStrTop + u8".fig").toBasicString())));
                    }
                    found = true;
                    path = modPath;
                    break;
                }
            }
        }

        if (!found)
            throw RuntimeError(FString(std::format("Could not find module `{}`", modPathStrTop.toBasicString())));

        bool found2 = false;

        for (size_t i = 1; i < pathVec.size(); ++i) // has next module
        {
            const FString &next = pathVec.at(i);
            modPath = modPath.parent_path(); // get the folder
            modPath = modPath / FString(next + u8".fig").toBasicString();
            if (fs::exists(modPath))
            {
                if (i != pathVec.size() - 1)
                    throw RuntimeError(FString(std::format(
                        "expects {} as parent directory and find next module, but got a file", next.toBasicString())));
                // it's the last module
                found2 = true;
                path = modPath;
                break;
            }
            // `next` is a folder
            modPath = modPath.parent_path() / next.toBasicString();
            if (!fs::exists(modPath))
                throw RuntimeError(FString(std::format("Could not find module `{}`", next.toBasicString())));
            if (i == pathVec.size() - 1)
            {
                // `next` is the last module
                modPath = modPath / FString(next + u8".fig").toBasicString();
                if (!fs::exists(modPath))
                {
                    throw RuntimeError(FString(std::format(
                        "expects {} as parent directory and find next module, but got a file", next.toBasicString())));
                }
                found2 = true;
                path = modPath;
            }
        }

        if (!found2 && !fs::exists(modPath))
            throw RuntimeError(FString(std::format("Could not find module `{}`", pathVec.end()->toBasicString())));

        return path;
    }
};