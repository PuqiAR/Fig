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

Copyright (C) 2020-2026 PuqiAR

This software is licensed under the MIT License. See LICENSE.txt for details.
*/

#include <Utils/argparse/argparse.hpp>
#include <print>
#include <fstream>

#include <Core/core.hpp>
#include <Lexer/lexer.hpp>
#include <Parser/parser.hpp>
#include <Evaluator/evaluator.hpp>
#include <Utils/AstPrinter.hpp>
#include <Utils/utils.hpp>
#include <Error/errorLog.hpp>

static size_t addressableErrorCount = 0;
static size_t unaddressableErrorCount = 0;


int main(int argc, char **argv)
{
    argparse::ArgumentParser program("Fig Interpreter", Fig::Core::VERSION.data());
    program.add_argument("source")
        .help("source file to be interpreted")
        .default_value(std::string(""));
    // program.add_argument("-v", "--version")
    //     .help("get the version of Fig Interpreter")
    //     .default_value(false)
    //     .implicit_value(true);
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
    // if (program.get<bool>("--version"))
    // {
    //     std::print("Fig Interpreter version {}\n", Fig::Core::VERSION);
    //     return 0;
    // }
    Fig::FString sourcePath(program.get<std::string>("source"));
    if (sourcePath.empty())
    {
        std::cerr << "No source file provided.\n";
        return 1;
    }
    std::ifstream file(sourcePath.toBasicString());
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << sourcePath.toBasicString() << '\n';
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    Fig::Lexer lexer((Fig::FString(source)));

    // Token tok;
    // while ((tok = lexer.nextToken()).getType() != TokenType::EndOfFile)
    // {
    //     std::println("{}", tok.toString().toBasicString());
    // }

    Fig::Parser parser(lexer);
    std::vector<Fig::Ast::AstBase> asts;

    std::vector<FString> sourceLines = Fig::Utils::splitSource(Fig::FString(source));

    try
    {
        asts = parser.parseAll();
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

    // AstPrinter printer;
    // std::print("<Debug> AST:\n");
    // for (const auto &node : ast)
    // {
    //     printer.print(node);
    // }

    Fig::Evaluator evaluator;

    evaluator.SetSourcePath(sourcePath);
    evaluator.CreateGlobalContext();
    evaluator.RegisterBuiltinsValue(); 

    try
    {
        evaluator.Run(asts);
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
    catch (const std::exception &e)
    {
        std::cerr << "uncaught exception of: " << e.what() << '\n';
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
