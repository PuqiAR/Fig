/*!
    @file src/SourceManager/SourceManager.hpp
    @brief 源代码管理
    @author PuqiAR (im@puqiar.top)
    @date 2026-02-14
*/

#pragma once

#include <Deps/Deps.hpp>
#include <Core/SourceLocations.hpp>

#include <fstream>

namespace Fig
{
    class SourceManager
    {
    private:
        String filePath;
        String source;
        std::vector<String> lines;

    public:
        bool read = false;
        String &Read()
        {
            std::fstream fs(filePath.toStdString());
            if (!fs.is_open())
            {
                read = false;
                return source;
            }
            std::string line;
            while (std::getline(fs, line))
            {
                source += line + '\n';
                lines.push_back(String(line));
            }
            read = true;
            return source;
        }

        SourceManager() {}
        SourceManager(String _path) { filePath = std::move(_path); }

        bool HasLine(int64_t _line) const
        {
            return _line <= lines.size() && _line >= 1;
        }

        String GetLine(size_t _line) const
        {
            assert(_line <= lines.size() && "SourceManager: GetLine failed, index out of range");
            return lines[_line - 1];
        }

        String GetSub(size_t _index_start, size_t _length) const
        {
            return source.substr(_index_start, _length);
        }

        const String &GetSource() const
        {
            return source;
        }
    };
};