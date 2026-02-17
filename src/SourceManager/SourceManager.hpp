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
        std::vector<size_t> lineStartIndex; // 每行在整个源字符串中的起始 index

        void preprocessLineIndices()
        {
            lineStartIndex.clear();
            lineStartIndex.push_back(0);

            for (size_t i = 0; i < source.length(); ++i)
            {
                if (source[i] == U'\n')
                {
                    lineStartIndex.push_back(i + 1);
                }
                else if (source[i] == U'\r')
                {
                    // 处理 CRLF，只在 \n 处记录
                    if (i + 1 < source.length() && source[i + 1] == U'\n')
                        continue;

                    lineStartIndex.push_back(i + 1);
                }
            }
        }

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
            if (!source.empty() && source.back() == U'\n')
                source.pop_back();

            if (lines.empty())
            {
                lines.push_back(String()); // 填充一个空的
            }
            read = true;
            preprocessLineIndices();
            return source;
        }

        SourceManager() {}
        SourceManager(String _path) { filePath = std::move(_path); }

        bool HasLine(int64_t _line) const
        {
            return _line <= lines.size() && _line >= 1;
        }

        const String &GetLine(size_t _line) const
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

        std::pair<size_t, size_t> GetLineColumn(size_t index) const
        {
            if (lineStartIndex.empty())
            {
                return {1, 1};
            }

            // clamp index 到合法范围（Parser报错可能传入EOF位置）
            // size_t lastLine = lineStartIndex.size() - 1;
            if (index < lineStartIndex[0])
            {
                return {1, 1};
            }

            // upper_bound 找到第一个 > index 的行起点
            auto it = std::ranges::upper_bound(lineStartIndex.begin(), lineStartIndex.end(), index);

            size_t line;
            if (it == lineStartIndex.begin())
            {
                line = 0;
            }
            else
            {
                line = static_cast<size_t>(it - lineStartIndex.begin() - 1);
            }

            size_t column = index - lineStartIndex[line] + 1;
            return {line + 1, column};
        }
    };
};