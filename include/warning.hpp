#pragma once

#include <fig_string.hpp>

#include <magic_enum/magic_enum.hpp>
#include <unordered_map>

namespace Fig
{
    class Warning
    {
    private:
        size_t id; // the id (standard) of warning
        FString msg;
        size_t line, column;
    public:
        static const std::unordered_map<size_t, FString> standardWarnings;
        Warning(size_t _id, FString _msg)
        {
            id = _id;
            msg = std::move(_msg);
        }
        Warning(size_t _id, FString _msg, size_t _line, size_t _column)
        {
            id = _id;
            msg = std::move(_msg);
            line = _line;
            column = _column;
        }

        auto getIDName()
        {
            return standardWarnings.at(id);
        }

        auto getID()
        {
            return id;
        }
        auto getMsg()
        {
            return msg;
        }
        auto getLine()
        {
            return line;
        }
        auto getColumn()
        {
            return column;
        }

    };

};