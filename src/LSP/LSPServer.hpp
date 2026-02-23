#pragma once

#include <Ast/Ast.hpp>
#include <Lexer/Lexer.hpp>
#include <Parser/Parser.hpp>
#include <Sema/Analyzer.hpp>

#include <Error/Error.hpp>
#include <SourceManager/SourceManager.hpp>

#include <charconv> // C++17/20 字符串转数字
#include <iostream>
#include <string>

#include <Utils/json/json.hpp>
using json = nlohmann::json;

namespace Fig
{

    class LspServer
    {
    public:
        void Run()
        {
            std::ios_base::sync_with_stdio(false);
            std::cin.tie(NULL);

            while (true)
            {
                std::string line;
                if (!std::getline(std::cin, line))
                    break; // 退出循环

                // 解析 HTTP 风格的 Header: "Content-Length: 123\r"
                if (line.starts_with("Content-Length: "))
                {
                    std::size_t length = 0;
                    // 使用 C++ 极速的 from_chars 替代 stoi
                    auto res = std::from_chars(line.data() + 16, line.data() + line.size(), length);
                    if (res.ec != std::errc())
                        continue;

                    // 消费 Header 和 Body 之间那行空的 "\r\n"
                    std::string emptyLine;
                    std::getline(std::cin, emptyLine);

                    // 读取对应字节的 JSON Body
                    std::string jsonBody(length, '\0');
                    std::cin.read(jsonBody.data(), length);

                    // 派发给路由
                    HandleMessage(jsonBody);
                }
            }
        }

    private:
        void HandleMessage(const std::string &rawJson)
        {
            json req = json::parse(rawJson, nullptr, false);
            if (req.is_discarded())
                return; // 忽略解析失败的报文

            std::string method = req.value("method", "");

            if (method == "initialize")
            {
                Respond(req["id"], R"({
                    "capabilities": {
                        "textDocumentSync": 1,
                        "hoverProvider": true
                    }
                })");
            }
            else if (method == "textDocument/didOpen")
            {
                std::string uri  = req["params"]["textDocument"]["uri"];
                std::string text = req["params"]["textDocument"]["text"];
                PublishDiagnostics(uri, text);
            }
            else if (method == "textDocument/didChange")
            {
                std::string uri = req["params"]["textDocument"]["uri"];
                std::string text = req["params"]["contentChanges"][0]["text"];
                PublishDiagnostics(uri, text);
            }
        }

        void Respond(int id, const std::string &resultJsonString)
        {
            std::string response = "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(id)
                                   + ",\"result\":" + resultJsonString + "}";
            std::cout << "Content-Length: " << response.size() << "\r\n\r\n"
                      << response << std::flush;
        }

        void SendDiagnostics(const std::string &uri, const Error *err = nullptr)
        {
            json diagnostics = json::array(); // 默认空数组，代码无错时擦除红线

            if (err)
            {
                // LSP 规定行列号必须从 0 开始算
                int startLine = err->location.sp.line - 1;
                int startChar = err->location.sp.column - 1;
                int endLine   = startLine;
                int endChar   = startChar + err->location.sp.tok_length;

                std::string fullMessage = err->message.toStdString();
                if (!err->suggestion.empty())
                {
                    fullMessage += "  💡suggestion: " + err->suggestion.toStdString();
                }

                diagnostics.push_back(
                    {{"range",
                         {{"start", {{"line", startLine}, {"character", startChar}}},
                             {"end", {{"line", endLine}, {"character", endChar}}}}},
                        {"severity", 1}, // 1 = 致命错误红线
                        {"source", "Fig LSP Server"},
                        {"message", fullMessage}});
            }

            // 组装 Notification
            json notification = {{"jsonrpc", "2.0"},
                {"method", "textDocument/publishDiagnostics"},
                {"params", {{"uri", uri}, {"diagnostics", diagnostics}}}};

            std::string response = notification.dump();
            std::cout << "Content-Length: " << response.size() << "\r\n\r\n"
                      << response << std::flush;
        }

        void PublishDiagnostics(const std::string &uri, const std::string &sourceCode)
        {
            SourceManager manager;
            manager.LoadFromMemory(sourceCode);

            Lexer  lexer(sourceCode, "");
            Parser parser(lexer, manager, "");

            // 1. 语法检查拦截
            auto parserResult = parser.Parse();
            if (!parserResult)
            {
                SendDiagnostics(uri, &parserResult.error());
                return;
            }

            Program *program = *parserResult;

            Analyzer analyzer(manager);

            // 语义检查拦截
            auto analyzerResult = analyzer.Analyze(program);
            if (!analyzerResult)
            {
                SendDiagnostics(uri, &analyzerResult.error());
                return;
            }

            // 3. 一切完美，发射空数组清空过去的错误红线
            SendDiagnostics(uri, nullptr);
        }
    };
} // namespace Fig