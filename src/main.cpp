/*
 ███████████ █████   █████ ██████████    ███████████ █████   █████████     █████         █████████   ██████   █████   █████████  █████  █████   █████████     █████████  ██████████
░█░░░███░░░█░░███   ░░███ ░░███░░░░░█   ░░███░░░░░░█░░███   ███░░░░░███   ░░███         ███░░░░░███ ░░██████ ░░███   ███░░░░░███░░███  ░░███   ███░░░░░███   ███░░░░░███░░███░░░░░█
░   ░███  ░  ░███    ░███  ░███  █ ░     ░███   █ ░  ░███  ███     ░░░     ░███        ░███    ░███  ░███░███ ░███  ███     ░░░  ░███   ░███  ░███    ░███  ███     ░░░  ░███  █ ░
    ░███     ░███████████  ░██████       ░███████    ░███ ░███             ░███        ░███████████  ░███░░███░███ ░███          ░███   ░███  ░███████████ ░███          ░██████
    ░███     ░███░░░░░███  ░███░░█       ░███░░░█    ░███ ░███    █████    ░███        ░███░░░░░███  ░███ ░░██████ ░███    █████ ░███   ░███  ░███░░░░░███ ░███    █████ ░███░░█
    ░███     ░███    ░███  ░███ ░   █    ░███  ░     ░███ ░░███  ░░███     ░███      █ ░███    ░███  ░███  ░░█████ ░░███  ░░███  ░███   ░███  ░███    ░███ ░░███  ░░███  ░███ ░   █
    █████    █████   █████ ██████████    █████       █████ ░░█████████     ███████████ █████   █████ █████  ░░█████ ░░█████████  ░░████████   █████   █████ ░░█████████  ██████████
   ░░░░░    ░░░░░   ░░░░░ ░░░░░░░░░░    ░░░░░       ░░░░░   ░░░░░░░░░     ░░░░░░░░░░░ ░░░░░   ░░░░░ ░░░░░    ░░░░░   ░░░░░░░░░    ░░░░░░░░   ░░░░░   ░░░░░   ░░░░░░░░░  ░░░░░░░░░░


                                                                                    .---.
           .              __.....__                         .--.                    |   |             _..._                                               __.....__
         .'|          .-''         '.                  _.._ |__|  .--./)            |   |           .'     '.   .--./)                        .--./)  .-''         '.
     .| <  |         /     .-''"'-.  `.              .' .._|.--. /.''\\             |   |          .   .-.   . /.''\\                        /.''\\  /     .-''"'-.  `.
   .' |_ | |        /     /________\   \             | '    |  || |  | |            |   |    __    |  '   '  || |  | |                 __   | |  | |/     /________\   \
 .'     || | .'''-. |                  |           __| |__  |  | \`-' /             |   | .:--.'.  |  |   |  | \`-' /      _    _   .:--.'.  \`-' / |                  |
'--.  .-'| |/.'''. \\    .-------------'          |__   __| |  | /("'`              |   |/ |   \ | |  |   |  | /("'`      | '  / | / |   \ | /("'`  \    .-------------'
   |  |  |  /    | | \    '-.____...---.             | |    |  | \ '---.            |   |`" __ | | |  |   |  | \ '---.   .' | .' | `" __ | | \ '---. \    '-.____...---.
   |  |  | |     | |  `.             .'              | |    |__|  /'""'.\           |   | .'.''| | |  |   |  |  /'""'.\  /  | /  |  .'.''| |  /'""'.\ `.             .'
   |  '.'| |     | |    `''-...... -'                | |         ||     ||          '---'/ /   | |_|  |   |  | ||     |||   `'.  | / /   | |_||     ||  `''-...... -'
   |   / | '.    | '.                                | |         \'. __//                \ \._,\ '/|  |   |  | \'. __// '   .'|  '/\ \._,\ '/\'. __//
   `'-'  '---'   '---'                               |_|          `'---'                  `--'  `" '--'   '--'  `'---'   `-'  `--'  `--'  `"  `'---'

Copyright (C) 2020-2025 PuqiAR

This software is licensed under the MIT License. See LICENSE.txt for details.
*/

#include <argparse/argparse.hpp>
#include <print>
#include <fstream>

#include <core.hpp>
#include <lexer.hpp>
#include <parser.hpp>
#include <evaluator.hpp>
#include <AstPrinter.hpp>
#include <errorLog.hpp>

static size_t addressableErrorCount = 0;
static size_t unaddressableErrorCount = 0;

std::vector<FString> splitSource(FString source)
{
    UTF8Iterator it(source);
    std::vector<FString> lines;
    FString currentLine;
    while (!it.isEnd())
    {
        UTF8Char c = *it;
        if (c == U'\n')
        {
            lines.push_back(currentLine);
            currentLine = FString(u8"");
        }
        else
        {
            currentLine += c.getString();
        }
        ++it;
    }
    if (!currentLine.empty())
    {
        lines.push_back(currentLine);
    }
    return lines;
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser program("Fig Interpreter", Fig::Core::VERSION.data());
    program.add_argument("source")
        .help("source file to be interpreted");
    // interpreter

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    Fig::FString sourcePath(program.get<std::string>("source"));
    std::ifstream file(sourcePath.toBasicString());
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << sourcePath.toBasicString() << '\n';
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Fig::Lexer lexer((Fig::FString(source)));
    Fig::Parser parser(lexer);
    std::vector<Fig::Ast::AstBase> ast;

    std::vector<FString> sourceLines = splitSource(Fig::FString(source));

    try
    {
        ast = parser.parseAll();
    }
    catch (const Fig::AddressableError &e)
    {
        addressableErrorCount++;
        ErrorLog::logAddressableError(e, sourcePath, sourceLines);
        return 1;
    }
    catch (const Fig::UnaddressableError &e)
    {
        unaddressableErrorCount++;
        ErrorLog::logUnaddressableError(e);
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "uncaught exception of: " << e.what() << '\n';
        return 1;
    }

    // Token tok;
    // while ((tok = lexer.nextToken()).getType() != TokenType::EndOfFile)
    // {
    //     std::println("{}", tok.toString().toBasicString());
    // }

    // AstPrinter printer;
    // std::print("<Debug> AST:\n");
    // for (const auto &node : ast)
    // {
    //     printer.print(node);
    // }

    Fig::Evaluator evaluator(ast);
    try
    {
        evaluator.run();
    }
    catch (const Fig::AddressableError &e)
    {
        addressableErrorCount++;
        ErrorLog::logAddressableError(e, sourcePath, sourceLines);
        evaluator.printStackTrace();
        return 1;
    }
    catch (const Fig::UnaddressableError &e)
    {
        unaddressableErrorCount++;
        ErrorLog::logUnaddressableError(e);
        evaluator.printStackTrace();
        return 1;
    }

    // try
    // {
    //     std::vector<Fig::Ast> ast = parser.parseAll();
    //     AstPrinter printer;

    //     std::print("<Debug> AST:\n");
    //     for (const auto &node : ast)
    //     {
    //         printer.print(node);
    //     }

    //     Fig::Evaluator evaluator(ast);
    //     evaluator.run();
    // }
    // catch (const Fig::AddressableError &e)
    // {
    //     std::cerr << e.what() << '\n';
    //     return 1;
    // }
    // catch (const Fig::UnaddressableError &e)
    // {
    //     std::cerr << e.what() << '\n';
    //     return 1;
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << e.what() << '\n';
    //     return 1;
    // }
}
