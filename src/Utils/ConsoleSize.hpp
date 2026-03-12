#include <optional>
#include <utility>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

namespace Fig::Utils
{

    /**
     * 获取控制台窗口的大小（行数和列数）
     * @return 如果成功，返回包含 (rows, cols) 的 optional；失败返回 std::nullopt
     */
     
    inline std::optional<std::pair<int, int>> getConsoleSize()
    {
#ifdef _WIN32
        // Windows: GetConsoleScreenBufferInfo
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        HANDLE                     hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        if (hConsole == INVALID_HANDLE_VALUE)
        {
            return std::nullopt;
        }

        if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        {
            return std::nullopt;
        }

        // 窗口大小 = 右下角坐标 - 左上角坐标 + 1
        int cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

        return std::make_pair(rows, cols);

#else
        // Linux / macOS / Unix: ioctl
        struct winsize w;

        // 标准输出被重定向到文件 ? 
        if (!isatty(STDOUT_FILENO))
        {
            // 不是终端，获取环境变量
            char *cols_env = getenv("COLUMNS");
            char *rows_env = getenv("LINES");
            if (cols_env && rows_env)
            {
                int cols = std::stoi(cols_env);
                int rows = std::stoi(rows_env);
                return std::make_pair(rows, cols);
            }
            return std::nullopt;
        }

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1)
        {
            return std::nullopt;
        }

        int cols = w.ws_col;
        int rows = w.ws_row;

        // 如果 ws_col 或 ws_row 为 0，使用环境变量
        if (cols == 0 || rows == 0)
        {
            char *cols_env = getenv("COLUMNS");
            char *rows_env = getenv("LINES");
            if (cols_env)
                cols = std::stoi(cols_env);
            if (rows_env)
                rows = std::stoi(rows_env);
        }

        if (cols > 0 && rows > 0)
        {
            return std::make_pair(rows, cols);
        }

        return std::nullopt;
#endif
    }

} // namespace Fig::Utils