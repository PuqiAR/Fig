#include <Parser/Parser.hpp>
#include <Sema/Analyzer.hpp>
#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;

void runTest(const std::string &path)
{
    using namespace Fig;
    std::cout << "\n[TEST] Testing: " << path << std::endl;

    SourceManager srcManager{String(path)};
    String        source = srcManager.Read();
    if (!srcManager.read)
    {
        std::cerr << "FAILED: Could not read file" << std::endl;
        return;
    }

    Lexer  lexer(source, String(path));
    Parser parser(lexer, srcManager, String(path));

    auto pRes = parser.Parse();
    if (!pRes)
    {
        std::cerr << "FAILED: Parser Error" << std::endl;
        ReportError(pRes.error(), srcManager);
        return;
    }

    // 修复：确保 analyzer 存活直到错误打印完成
    Analyzer analyzer(srcManager);
    auto     aRes = analyzer.Analyze(*pRes);

    if (!aRes)
    {
        std::cout << "SUCCESS: Analyzer correctly caught error:" << std::endl;
        ReportError(aRes.error(), srcManager);
    }
    else
    {
        std::cerr << "FAILED: Analyzer missed the semantic error!" << std::endl;
    }
}

int main()
{
    std::string testDir = "T:/Files/Maker/Code/MyCodingLanguage/The Fig Project/Fig/tests/Sema";
    for (const auto &entry : fs::directory_iterator(testDir))
    {
        if (entry.path().extension() == ".fig")
        {
            runTest(entry.path().string());
        }
    }
    return 0;
}
