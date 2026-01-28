#pragma once

#include <Core/core.hpp>
#include <Lexer/lexer.hpp>
#include <Parser/parser.hpp>
#include <Evaluator/evaluator.hpp>
#include <Utils/AstPrinter.hpp>
#include <Utils/utils.hpp>
#include <Error/errorLog.hpp>
#include <Core/runtimeTime.hpp>

#include <format>
#include <iostream>
#include <ostream>
#include <string>

namespace Fig
{
    class Repl
    {
    private:
        std::istream &istream;
        std::ostream &ostream;

    public:
        Repl(std::istream &_istream = std::cin, std::ostream &_ostream = std::cout) :
            istream(_istream), ostream(_ostream)
        {
        }

        FString readline() const
        {
            std::string buf;
            std::getline(istream, buf);

            return FString(buf);
        }

        static std::string getPrompt()
        {
            static const std::string prompt = std::format("Fig {} ({})[{} {}-bit on `{}`]\n",
                                                          Core::VERSION,
                                                          Core::COMPILE_TIME,
                                                          Core::COMPILER,
                                                          Core::ARCH,
                                                          Core::PLATFORM)
                                              + "Type '!exit' to exit\n" + "feel free to type!";
            return prompt;
        }

        void Start() const;
    };

}; // namespace Fig