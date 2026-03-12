#include <Repl/Repl.hpp>
#include <Core/Core.hpp>

int main()
{
    using namespace Fig;
    Repl repl(CoreIO::GetStdCin(), CoreIO::GetStdOut(), CoreIO::GetStdErr());

    repl.Start();
}