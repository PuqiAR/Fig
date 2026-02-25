#include <LSP/LSPServer.hpp>

#ifdef _WIN32
    #include <fcntl.h>
    #include <io.h>
#endif

int main()
{
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    Fig::LspServer server;

    server.Run();
    return 0;
}